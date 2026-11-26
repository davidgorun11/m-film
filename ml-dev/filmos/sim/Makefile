# film OS simulator: make && ./out/sim scenes/shelf.txt out/frames/shelf
SRC = sim.c port_files.c fonts.c films.c ../fos_gfx.c ../fos_film.c ../fos_ui.c ../fos_shelf.c ../fos_pages.c ../fos_hud.c ../fos_print.c \
      ../fos_dark.c ../fos_dev.c ../fos_lab.c ../fos_looks.c ../fos_jpeg.c
CFLAGS = -O2 -g -Wall -Wextra -Wno-unused-parameter -Wdouble-promotion -std=gnu99
# M-Film's version, as the camera build gets it (VERSION at the top of the repository)
CFLAGS += -DMFILM_VERSION='"$(shell tr -d " \r\n" < ../../../VERSION)"'

out/sim: $(SRC) ../*.h out/FOSFONT.BIN out/FOSFILMS.BIN
	mkdir -p out
	$(CC) $(CFLAGS) -o $@ $(SRC) -lm

# the fonts, as the camera finds them on its card (ML/DATA/FOSFONT.BIN)
out/FOSFONT.BIN: ../fos_font.c ../tools/fontblob.c ../fos_gfx.h
	mkdir -p out
	$(CC) -O2 -I.. -o out/fontblob ../tools/fontblob.c
	./out/fontblob $@

# the films beyond the built-in ones, as the camera finds them on its card (ML/DATA/FOSFILMS.BIN)
out/FOSFILMS.BIN: ../tools/genfilms.py ../tools/catalog.py ../../films/films.py ../../films/filmluts.py ../fos_film.c
	mkdir -p out
	python3 ../tools/genfilms.py $@

# every film stock's colour table, as the build puts them in the card's ML/DARKROOM: the camera makes
# none itself (mkcard.sh copies these onto the simulator's cards). --plain: without the Film-Luts LUTs
# (films/getluts.py fetches them) those film stocks get the table of their look, which is enough here
out/tables/TABLES.txt: out/devtest out/FOSFILMS.BIN ../tools/gentables.py ../tools/genfilms.py ../../films/lut.py \
		../../films/filmshape.py ../../films/measure.py ../../films/spektra.py ../../films/spektra.sha256 $(wildcard ../../films/spektra/*.tab) \
		$(wildcard ../../references/*/record.txt ../../references/*/look.txt)
	python3 ../tools/gentables.py out/tables --films out/FOSFILMS.BIN --devtest out/devtest --plain

# the same with the big UI of a low resolution screen (the 1100D's): make out/sim-big
out/sim-big: $(SRC) ../*.h out/FOSFONT.BIN out/FOSFILMS.BIN
	mkdir -p out
	$(CC) $(CFLAGS) -DFOS_BIGUI -DFOS_KEY_DISP -o $@ $(SRC) -lm

clean:
	rm -rf out

# developing film: ./out/devtest [-f film] [-r raw.u16] in.CR2 out.FOP out.ppm
DEV = devtest.c port_files.c films.c ../fos_dev.c ../fos_print.c ../fos_looks.c ../fos_film.c ../fos_gfx.c ../fos_jpeg.c
out/devtest: $(DEV) ../*.h out/FOSFILMS.BIN
	mkdir -p out
	$(CC) $(CFLAGS) -DFOS_DEV_TEST -o $@ $(DEV) -lm

# test CR2 files (mkcard.sh builds its own; colourtest.py wants this one)
out/mkcr2: tools/mkcr2.c
	mkdir -p out
	$(CC) -O2 -o $@ tools/mkcr2.c -lm

# the same, loading words off their boundary the way the camera's ARM does (rotated):
# a print that differs from out/devtest's has a read the camera gets wrong (lines, #6)
out/devtest-arm: $(DEV) ../*.h out/FOSFILMS.BIN
	mkdir -p out
	$(CC) $(CFLAGS) -DFOS_DEV_TEST -DFOS_ARM_UNALIGNED -o $@ $(DEV) -lm

# the simulator and devtest with AddressSanitizer and UndefinedBehaviorSanitizer (and gcc's
# bounds-strict, where the compiler has it): each stops at the first read or write outside what it
# has, or an int running over, and says where; memory it never gives back is said as it ends
# (./cardtest.sh --sanitize). gcc's undefined leaves out the two float checks: a float divided by 0,
# and a float no int holds made an int, which is INT_MIN on a PC and the nearest int on the camera, so
# the two print differently. sim-san and devtest-san are built again when the Makefile changes as
# well: built with its old flags, they would pass for the new ones
SAN = -fsanitize=address,undefined,float-cast-overflow,float-divide-by-zero \
      -fno-sanitize-recover=all -fno-omit-frame-pointer \
      $(shell $(CC) -fsanitize=bounds-strict -x c -c /dev/null -o /dev/null 2>/dev/null && echo -fsanitize=bounds-strict)
out/sim-san: $(SRC) ../*.h out/FOSFONT.BIN out/FOSFILMS.BIN Makefile
	mkdir -p out
	$(CC) $(CFLAGS) $(SAN) -o $@ $(SRC) -lm
out/devtest-san: $(DEV) ../*.h out/FOSFILMS.BIN Makefile
	mkdir -p out
	$(CC) $(CFLAGS) $(SAN) -DFOS_DEV_TEST -o $@ $(DEV) -lm

# the grain the darkroom draws: the film stock's strength at every clump width, its clumps as
# wide as its grain_size says, and no pattern of their own (#104): ./out/graintest
out/graintest: graintest.c port_files.c ../fos_dev.c ../fos_print.c ../fos_looks.c ../fos_film.c ../fos_gfx.c ../fos_jpeg.c ../*.h
	mkdir -p out
	$(CC) $(CFLAGS) -DFOS_DEV_TEST -o $@ graintest.c port_files.c ../fos_dev.c ../fos_print.c ../fos_looks.c ../fos_film.c ../fos_gfx.c ../fos_jpeg.c -lm
