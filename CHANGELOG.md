# Changes in this fork

What changed in M-Film for the Canon EOS 6D Mark II, newest first. For what the fork adds to the original
M-Film as a whole, see the [README](README.md).

## Next (not released yet)

From the original M-Film:

- **The darkroom develops with the white balance the camera was on** (#123). It used one fixed balance for
  every frame, which left a daylight grey warm before the film did anything. Loading a roll now sets the
  camera to the light the film is balanced for, 5500 K or 3200 K, with no shift.
- **The Film-Luts films get a finished photo's tone** (#125): their tables were made on finished photos, and
  they now get that contrast instead of the plain sRGB curve.
- **A highlight is no longer tinted by its own glow, and every film's glow is fainter** (#154). A bright wall
  or a white sky kept a red cast from the halation; now the glow falls only on what lies beside a highlight.
- **The four cinema print films have left the shelf** (#126): Kodak Vision 2383 and 2393, Fujifilm Eterna
  3510 and 3513. Their tables came with no licence and were made for a film scan, not a photo. A roll already
  shot on one stays on the card and is not developed as another film. If one is loaded, rewind it first.
- **Grain of Portra 400, Portra 400 NC/VC/UC and Pro 400H** is the original's again.
- **Five films' lines on the shelf** say what they are: CineStill 400T is M-Film's own look, the two infrared
  films are impressions (#149).
- The Film-Luts tables are fetched at one fixed commit and each is checked before it is baked (#155).

With these, every film's print from this fork is pixel for pixel the original's, Small and Large, checked on
all 68 films with the same RAW.

## 0.2.1-6d2.2 (2026-11-15)

### Faster developing

A whole roll develops about a third faster than in 0.2.1-6d2.1. Measured on the same camera, card and roll
(5 RAW frames, Large prints):

| | 0.2.1-6d2.1 | 0.2.1-6d2.2 |
|---|---|---|
| The roll | 180 s | 117 s (−35 %) |
| One frame | 32–38 s | 21–25 s |
| Its JPEG | about 6 s | about 3 s |

- **The Large and Medium print uses both cores.** The DIGIC 7 has two processor cores and M-Film used one.
  A helper task on the second core now takes half of every band of the print: the colour, the grain and the
  JPEG. Each core encodes its own half of the JPEG completely, and the halves are joined with the JPEG
  standard's restart markers. Pixels are exactly the same as before; the file is about 1 % bigger.
- **Zoom in Prints is a setting, off by default** (System → *Zoom in Prints*). The zoom file kept the second
  core busy for about 3.5 s a frame and took 6.5 MB of the card. Turn it on to look closer at a print in
  Prints.
- **Prints:** the next print shows at once, its colours together with its pixels.
- **The JPEG's EXIF** comes right after the start of the file, with no JFIF block ahead of it.

### Opening and leaving M-Film

- **No shutter animation** when M-Film opens or closes: it opens straight on its screen and closes at once.
- **No flash of the last photo.** Under M-Film the camera keeps Canon's playback open, with the last photo
  in it. When M-Film opened or closed, that photo and Canon's numbers around it showed for a moment. The
  screen now stays black in between.
- **The boot animation is a setting, off by default** (System → *Boot animation*).
- **Opening from LiveView** no longer leaves a black screen until a button is pressed.
- **LiveView goes off when M-Film opens** and stays off when it closes. The LV button turns it back on.

### Shooting screen and buttons

- **No HUD on the 6D Mark II.** While you shoot you see Canon's own screen. Canon's screen can't be kept from
  drawing over M-Film's readout on this camera, and the two flickered. The top plate, the Viewfinder page
  and the wind lever are gone with it.
- **PLAY on the shooting screen opens Canon's playback** by default. *PLAY shows prints only* in System
  makes it open M-Film's Prints instead.
- **Trash erases in Canon's playback again**, with or without LiveView.
- **Settings:** SET works on every row, a choice turns round with the arrows, On/Off looks like the other
  choices, and the right arrow runs an action.
- **Lens page:** an electronic lens is read from the lens, not set by hand. It shows the lens's focal range,
  and the focal length when LiveView was on to measure it.

### Darkroom

- **No copy of the JPEG in Canon's folder.** Canon's playback hides a JPEG that has the same number as a RAW,
  so the copy couldn't sit next to its frame. The developed JPEGs are in their own `DCIM` folder.
- **A frame's RAW is found in any `DCIM` folder**, not only in the folder where the roll started.

### Films (from the original M-Film)

- The colour tables of 59 films are Pat David's and are CC BY-SA 4.0; this is now stated
  ([THIRD_PARTY.md](THIRD_PARTY.md)).
- The four cinema print films get the Cineon log input their tables were made for.
- Finer grain for Portra 400, Pro 400H and Portra 400 NC/VC/UC.
- A frame clips where its own samples clip.
- Every film's colour table is made in one place; the camera makes none.

### Install instructions

- **After formatting the card, run the firmware update again** (MENU → *Firmware ver.*). The README used to
  say it was needed once per camera, but formatting in the camera clears the card's mark to start Magic
  Lantern, and without it the camera starts without M-Film.

### Log

- `ML/FILMROLL.LOG` notes the screen state and Canon's auto power off, and writes "startup waiting" only when
  something changes.

## 0.2.1-6d2.1 (2026-11-11)

The first release for the 6D Mark II: M-Film running on DIGIC 7, the film OS as a module, faster RAW
reading, Small/Medium/Large prints, the date stamp, M-Film without LiveView and one button scheme on every
screen. See the [release](../../releases/tag/v0.2.1-6d2.1).
