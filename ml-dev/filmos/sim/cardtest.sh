#!/usr/bin/env bash
# The darkroom on a card of problem rolls (mkcard.sh --problems): it develops
# what it can, and says why it can't the rest; Delete roll takes exactly
# what it says, and what it couldn't is still there to delete again; a card
# that can't take long folder names keeps no empty ones (#65); a roll
# developed again keeps its folder; a broken RAW fails and says why, or
# develops as far as it goes, and the sample RAW develops. Fails if a line
# is missing or a file is wrong.
#   ./cardtest.sh
#   ./cardtest.sh --sanitize   the same with the simulator and devtest built with ASan and UBSan
#                              (out/sim-san, out/devtest-san): anything they find fails it
set -euo pipefail
# the rolls are developed with Zoom in Prints on, so each print has its zoom file (Fnn.ZOM) for Delete roll
# to take: this build has the setting off by default (DG11's build)
export FOS_ZOOM=1
cd "$(dirname "$0")"
case "${1:-}" in
    "") san= ;;
    --sanitize) san=-san ;;
    *) echo "usage: $0 [--sanitize]" >&2; exit 2 ;;
esac
make -s "out/sim$san" "out/devtest$san" out/mkcr2
sim=./out/sim$san devtest=./out/devtest$san
# what the sanitizers say: a report's first line, the summary at its end, or that they broke down
san_report='runtime error: |(ERROR|SUMMARY): [A-Za-z]+Sanitizer|Sanitizer:? (CHECK failed|has encountered a fatal error)'
# and what a program they stop ends with: not their usual 1, devtest's for a frame it can't develop
san_exit=99
if [ -n "$san" ]; then
    # what they find stops the program (the Makefile's -fno-sanitize-recover; a Mac's would abort
    # it), a leak as it ends, and the stack goes in the log
    san_opts=exitcode=$san_exit:abort_on_error=0
    export ASAN_OPTIONS=$san_opts UBSAN_OPTIONS=$san_opts:print_stacktrace=1
    # an ordinary build in their place would find nothing, and pass
    for f in "$sim" "$devtest"; do
        syms=$(nm "$f")
        [[ $syms == *__asan_init* && $syms == *__ubsan_handle_* ]] \
            || { echo "FAIL  $f is not built with ASan and UBSan: remove it and run this again"; exit 1; }
    done
fi
CARD=out/cardp ./mkcard.sh --problems >/dev/null
rm -rf out/frames/problems out/cardtest-gap.log out/cardtest-gone.log && mkdir -p out/frames/problems
log=out/cardtest.log
# what they find in the simulator stops it, and the test with it: on the way out of a failed test,
# the first report in each log, and where
trap 'st=$?
      [ $st = 0 ] || grep -m1 -hE -A15 "$san_report" "$log" out/cardtest-gap.log out/cardtest-gone.log 2>/dev/null || :
      [ $st != $san_exit ] || echo "cardtest: ASan or UBSan stopped the simulator (log: sim/out/cardtest*.log)"
      exit $st' EXIT
FOS_CARD=out/cardp/ "$sim" scenes/darkroom_problems.txt out/frames/problems > "$log" 2>&1

fail=0
expect() {
    if grep -qF -- "$2" "$log"; then echo "ok    $1"
    else echo "FAIL  $1: no \"$2\""; fail=1; fi
}
reject() {        # an extended regex
    if grep -qE -- "$2" "$log"; then echo "FAIL  $1: \"$2\""; fail=1
    else echo "ok    $1"; fi
}

# a roll shot as JPEG: each frame says so, and names the JPEG
expect "JPEG roll: frame 1 is a JPEG"       "roll 006 frame 1: shot as JPEG, no RAW: out/cardp/DCIM/100CANON/IMG_0501.JPG"
expect "JPEG roll: frame 4 is a JPEG"       "roll 006 frame 4: shot as JPEG, no RAW: out/cardp/DCIM/100CANON/IMG_0504.JPG"
# Adobe RGB (_MG_): found by Canon's name; the deleted one is named that way too
expect "Adobe RGB: frame 1 found"           "roll 007 frame 1 developed"
expect "Adobe RGB: deleted, by Canon's name" "roll 007 frame 2: no RAW file out/cardp/DCIM/100CANON/_MG_0506.CR2"
expect "Adobe RGB: frame 4 found"           "roll 007 frame 4 developed"
# across IMG_9999 -> IMG_0001: each frame in its own folder's log
expect "9999 wrap: all four RAWs"           "roll 008 (gold200): 4 frames, 4 RAW files"
reject "9999 wrap: nothing missing"         "roll 008 frame [0-9]+: (no RAW|shot as JPEG)"
for f in 1 2 3 4; do expect "9999 wrap: frame $f developed" "roll 008 frame $f developed"; done

# Delete roll, on the card just developed: the files that go are exactly
# the ones listed, and nothing else changes. (The rolls were developed at the
# simulator's print size, medium: each print has its zoom file, Fnn.ZOM.)
listing() { (cd "$1" && find . | grep -v '/LOOK[0-9]*\.LUT$' | sort); }
dcard=out/cardp
deletes() {     # deletes <what> <scene> <path that goes>...   (on $dcard)
    local what=$1 scene=$2; shift 2
    listing $dcard > out/del-before.txt
    FOS_CARD=$dcard/ "$sim" "scenes/$scene.txt" - >> "$log" 2>&1
    listing $dcard > out/del-after.txt
    printf '%s\n' "$@" | sed '/^$/d' | sort > out/del-want.txt
    comm -23 out/del-before.txt out/del-after.txt > out/del-gone.txt
    comm -13 out/del-before.txt out/del-after.txt > out/del-new.txt
    if cmp -s out/del-want.txt out/del-gone.txt && [ ! -s out/del-new.txt ]; then echo "ok    $what"
    else
        echo "FAIL  $what"; fail=1
        diff out/del-want.txt out/del-gone.txt | sed 's/^</  should have gone:/; s/^>/  shouldn'"'"'t have gone:/' | grep '^ ' || true
        sed 's/^/  new: /' out/del-new.txt
    fi
}
# asked, then not confirmed: nothing goes
deletes "Delete roll: not confirmed" delete_cancel ""
# prints only (007, Adobe RGB, frame 2 deleted): its JPEG folder and the darkroom's prints
deletes "Delete roll: prints only" delete_prints \
    "./DCIM/Oct1002 Ektar100" "./DCIM/Oct1002 Ektar100/ROLL007.TXT" "./DCIM/Oct1002 Ektar100/_MG_0505.JPG" \
    "./DCIM/Oct1002 Ektar100/_MG_0507.JPG" "./DCIM/Oct1002 Ektar100/_MG_0508.JPG" \
    ./ML/DARKROOM/R007 ./ML/DARKROOM/R007/F01.BMP ./ML/DARKROOM/R007/F03.BMP ./ML/DARKROOM/R007/F04.BMP \
    ./ML/DARKROOM/R007/F01.ZOM ./ML/DARKROOM/R007/F03.ZOM ./ML/DARKROOM/R007/F04.ZOM \
    ./ML/DARKROOM/R007/SHEET.BMP
expect "Delete roll: prints only, logged"   "lab: roll 007: deleted prints, 0 left"
# the whole roll (008, across IMG_9999) where IMG_9999's RAW and its print
# in DCIM can't be removed: the rest of it goes; its logs and the print
# folder's tag stay, so it is still there to delete again
FOS_KEEP=IMG_9999 deletes "Delete roll: whole roll, files that can't go" delete_whole \
    ./DCIM/100CANON/IMG_9998.CR2 ./DCIM/101CANON/IMG_0001.CR2 ./DCIM/101CANON/IMG_0002.CR2 \
    "./DCIM/Oct1003 Gold200/IMG_0001.JPG" "./DCIM/Oct1003 Gold200/IMG_0002.JPG" "./DCIM/Oct1003 Gold200/IMG_9998.JPG" \
    ./ML/DARKROOM/R008 ./ML/DARKROOM/R008/F01.BMP ./ML/DARKROOM/R008/F02.BMP ./ML/DARKROOM/R008/F03.BMP \
    ./ML/DARKROOM/R008/F04.BMP ./ML/DARKROOM/R008/SHEET.BMP \
    ./ML/DARKROOM/R008/F01.ZOM ./ML/DARKROOM/R008/F02.ZOM ./ML/DARKROOM/R008/F03.ZOM ./ML/DARKROOM/R008/F04.ZOM
expect "Delete roll: the RAW that can't go is said" "lab: can't remove out/cardp/DCIM/100CANON/IMG_9999.CR2"
expect "Delete roll: ... and counted"       "lab: roll 008: deleted all, 3 left"
# deleted again: the rest of it, the print folder and its tag, the RAW and
# its logs in both DCIM folders
deletes "Delete roll: whole roll across IMG_9999, again" delete_whole \
    "./DCIM/Oct1003 Gold200" "./DCIM/Oct1003 Gold200/IMG_9999.JPG" "./DCIM/Oct1003 Gold200/ROLL008.TXT" \
    ./DCIM/100CANON/IMG_9999.CR2 ./DCIM/100CANON/ROLL_008.TXT ./DCIM/101CANON/ROLL_008.TXT
# the roll in the camera (006 here): nothing goes
deletes "Delete roll: never the roll in the camera" delete_loaded ""
# the whole roll (007) once its prints are gone: straight to "sure?"; Canon's _MG_ names
deletes "Delete roll: whole roll in Adobe RGB" delete_whole \
    ./DCIM/100CANON/_MG_0505.CR2 ./DCIM/100CANON/_MG_0507.CR2 ./DCIM/100CANON/_MG_0508.CR2 ./DCIM/100CANON/ROLL_007.TXT
# the whole roll shot as JPEG (006): Canon's JPEGs are its frames
deletes "Delete roll: whole roll shot as JPEG" delete_whole \
    ./DCIM/100CANON/IMG_0501.JPG ./DCIM/100CANON/IMG_0502.JPG ./DCIM/100CANON/IMG_0503.JPG \
    ./DCIM/100CANON/IMG_0504.JPG ./DCIM/100CANON/ROLL_006.TXT \
    "./DCIM/Oct1001 Portra400" "./DCIM/Oct1001 Portra400/ROLL006.TXT" ./ML/DARKROOM/R006 ./ML/DARKROOM/R006/SHEET.BMP
n=$(grep -c "lab: can't remove" "$log" || true)
[ "$n" = 2 ] && echo "ok    Delete roll: nothing else it couldn't" || { echo "FAIL  Delete roll: $n \"can't remove\" lines, not 2"; fail=1; }

# A card that makes a long-named folder but can't write into it (a 550D's
# exFAT card, #65): the prints go in SEP0906-style folders, and no empty
# long-named one is left, however often the rolls are developed (6 and 7
# are developed again: they have frames that can't be)
long_dirs() { find "$1/DCIM" -mindepth 1 -maxdepth 1 -type d -name '* *' | wc -l | tr -d ' '; }    # (BSD wc pads)
CARD=out/cardl ./mkcard.sh --problems >/dev/null
for i in 1 2; do FOS_CARD=out/cardl/ FOS_LONG_DIRS_EMPTY=1 "$sim" scenes/darkroom_problems.txt - >> "$log" 2>&1; done
expect "Long names fail: short folder"      "darkroom: JPEGs in out/cardl/DCIM/OCT1003"
n=$(find out/cardl/DCIM/OCT1003 -name '*.JPG' | wc -l | tr -d ' ')
[ "$n" = 4 ] && echo "ok    Long names fail: 4 JPEGs in OCT1003" || { echo "FAIL  Long names fail: $n JPEGs in OCT1003"; fail=1; }
n=$(long_dirs out/cardl)
[ "$n" = 0 ] && echo "ok    Long names fail: no empty long folders" || { echo "FAIL  Long names fail: $n long folders left"; fail=1; }
# and a camera that can't remove a folder either: one left for each roll, not one more each time
CARD=out/cardl ./mkcard.sh --problems >/dev/null
for i in 1 2; do FOS_CARD=out/cardl/ FOS_LONG_DIRS_EMPTY=1 FOS_NO_RMDIR=1 "$sim" scenes/darkroom_problems.txt - >> "$log" 2>&1; done
n=$(long_dirs out/cardl)
[ "$n" = 3 ] && echo "ok    Long names fail, no folder removal: 3 left" || { echo "FAIL  Long names fail, no folder removal: $n left, not 3"; fail=1; }
# there, Delete roll still takes every file of the whole roll (008) and its
# logs: only the folders stay, empty
dcard=out/cardl FOS_NO_RMDIR=1 deletes "Delete roll, no folder removal: every file" delete_whole \
    ./DCIM/100CANON/IMG_9998.CR2 ./DCIM/100CANON/IMG_9999.CR2 ./DCIM/100CANON/ROLL_008.TXT \
    ./DCIM/101CANON/IMG_0001.CR2 ./DCIM/101CANON/IMG_0002.CR2 ./DCIM/101CANON/ROLL_008.TXT \
    ./DCIM/OCT1003/IMG_0001.JPG ./DCIM/OCT1003/IMG_0002.JPG ./DCIM/OCT1003/IMG_9998.JPG ./DCIM/OCT1003/IMG_9999.JPG \
    ./DCIM/OCT1003/ROLL008.TXT ./ML/DARKROOM/R008/F01.BMP ./ML/DARKROOM/R008/F02.BMP ./ML/DARKROOM/R008/F03.BMP \
    ./ML/DARKROOM/R008/F04.BMP ./ML/DARKROOM/R008/SHEET.BMP \
    ./ML/DARKROOM/R008/F01.ZOM ./ML/DARKROOM/R008/F02.ZOM ./ML/DARKROOM/R008/F03.ZOM ./ML/DARKROOM/R008/F04.ZOM
expect "Delete roll, no folder removal: the folders counted" "lab: roll 008: deleted all, 2 left"

# A roll developed again finds its folder whichever of the day's names it
# has: here "Oct1002 Ektar100 2", the first name free again (another roll
# of that day deleted since). No second folder, no frame developed again.
CARD=out/cardg ./mkcard.sh --problems >/dev/null
FOS_CARD=out/cardg/ "$sim" scenes/darkroom_problems.txt - >> "$log" 2>&1
mv "out/cardg/DCIM/Oct1002 Ektar100" "out/cardg/DCIM/Oct1002 Ektar100 2"
FOS_CARD=out/cardg/ "$sim" scenes/darkroom_problems.txt - > out/cardtest-gap.log 2>&1
cat out/cardtest-gap.log >> "$log"
grep -qF "darkroom: JPEGs in out/cardg/DCIM/Oct1002 Ektar100 2" out/cardtest-gap.log && [ ! -e "out/cardg/DCIM/Oct1002 Ektar100" ] \
    && echo "ok    A gap in the day's names: its own folder" || { echo "FAIL  A gap in the day's names: not its own folder"; fail=1; }
grep -qE "roll 007 frame [0-9]+ developed" out/cardtest-gap.log \
    && { echo "FAIL  A gap in the day's names: frames developed again"; fail=1; } || echo "ok    A gap in the day's names: nothing developed again"

# A roll shot on a film that has since left the shelf (upstream, the cinema print films,
# #126; this build keeps those, so a made-up film stands in): it is listed by the name in its log and not developed as some other
# film; its RAWs stay; the roll beside it develops
CARD=out/cardx ./mkcard.sh --gone >/dev/null
rm -rf out/frames/gone
FOS_CARD=out/cardx/ "$sim" scenes/darkroom_problems.txt out/frames/gone > out/cardtest-gone.log 2>&1
cat out/cardtest-gone.log >> "$log"
grep -qF "roll 004 frame 1: its film (kodakgone) is not on the shelf any more" out/cardtest-gone.log \
    && echo "ok    A film that left the shelf: the frame says so" || { echo "FAIL  A film that left the shelf: nothing said"; fail=1; }
if grep -qE "roll 004 frame [0-9]+ developed" out/cardtest-gone.log || ls out/cardx/ML/DARKROOM/R004/F*.BMP >/dev/null 2>&1; then
    echo "FAIL  A film that left the shelf: developed as another film"; fail=1
else echo "ok    A film that left the shelf: not developed as another film"; fi
n=$(find out/cardx/DCIM -name 'IMG_060[12].CR2' | wc -l | tr -d ' ')
[ "$n" = 2 ] && echo "ok    A film that left the shelf: its RAWs are still there" || { echo "FAIL  A film that left the shelf: $n of its 2 RAWs left"; fail=1; }
grep -qE "roll 005 frame 2 developed" out/cardtest-gone.log \
    && echo "ok    A film that left the shelf: the roll beside it develops" || { echo "FAIL  A film that left the shelf: roll 005 did not develop"; fail=1; }

# the sanitizers can say something without stopping the simulator (a leak, with LSAN_OPTIONS=exitcode=0)
[ -z "$san" ] || reject "The simulator: nothing said by ASan or UBSan" "$san_report"

# A broken RAW, as a damaged card or file can have: the frame fails and says
# why, or develops as far as the file goes, and it never reads or writes
# outside what it has, nor goes round forever. (A Huffman table with more codes
# than it has room for was written past the darkroom's lookup, a segment cut
# short was read past its end, and slices of no width went round forever.)
# tools/badcr2.py breaks a good frame in each way it knows; those it only takes
# apart and puts back, or changes in a way the darkroom must take, must still
# develop, every sample right, and those cut short, decoded to nonsense or read
# past their end develop. A frame still going after a minute fails (where
# there is a timeout command: a Mac has none). Reading or writing outside, an
# int running over or a float no int holds shows only with --sanitize; reading
# past the data but inside its buffer shows in neither, and can end differently
# in each (ASan fills new memory with 0xbe, where it is often 0 without it): CI
# runs both.
b=out/badraw
rm -rf "$b" && mkdir -p "$b"
# with the camera's white balance, so its maker note has the colour data as well
./out/mkcr2 "$b/good.CR2" --small --wb 2,1.5 --raw "$b/good.u16" 2>/dev/null
python3 tools/badcr2.py "$b/good.CR2" "$b"
develop() {
    if command -v timeout >/dev/null; then FOS_MIX=1 timeout 60 "$devtest" "$@"; else FOS_MIX=1 "$devtest" "$@"; fi
}
# frame <what> <want> <RAW> <out>   develops the RAW (<out>.fop, <out>.log). <want>: the message it must
#                                   fail with; "develops"; or "ok": it develops, every sample right
frame() {
    local what=$1 want=$2 raw=$3 out=$4 rc=0 found
    set -- "$raw" "$out.fop"
    [ "$want" != ok ] || set -- -r "$b/good.u16" "$@"
    develop "$@" > "$out.log" 2>&1 || rc=$?
    { echo "-- $what ($want)"; cat "$out.log"; } >> "$log"
    if found=$(grep -m1 -E "$san_report" "$out.log"); then echo "FAIL  $what: $found"; fail=1
    elif [ $rc = $san_exit ]; then echo "FAIL  $what: ASan or UBSan stopped it: $(tail -1 "$out.log")"; fail=1
    elif [ $rc = 124 ]; then echo "FAIL  $what: still going after a minute"; fail=1
    elif [ "$want" = ok ] || [ "$want" = develops ]; then
        if [ $rc = 0 ] && { [ "$want" = develops ] || grep -q "decoder: [0-9]* samples, 0 wrong" "$out.log"; }; then
            echo "ok    $what develops"
        else echo "FAIL  $what: $(tail -1 "$out.log") (exit $rc)"; fail=1; fi
    elif [ $rc = 0 ]; then echo "FAIL  $what developed"; fail=1
    elif [ $rc = 1 ] && grep -qxF "$raw: $want" "$out.log"; then echo "ok    $what: $want"
    else echo "FAIL  $what: $(tail -1 "$out.log") (exit $rc)"; fail=1; fi
}
while read -r name want; do frame "A broken RAW: $name" "$want" "$b/$name.CR2" "$b/$name"; done < "$b/LIST.txt"
# two of the widest frames print right across, not only mkcr2's 1296 columns at the left
expect "The widest frame: its picture worked out" "picture 260,16 16087x864"
expect "The widest frame: from edge to edge"      "picture 0,16 16384x864"
# and a whole RAW from a camera: the EOS M's sample
frame "The sample RAW" develops ../../films/sample/SAMPLE.CR2 "$b/sample"

[ $fail = 0 ] && echo "cardtest: all passed${san:+, with ASan and UBSan}" || { echo "cardtest: failed (log: sim/$log)"; exit 1; }
