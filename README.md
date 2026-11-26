# M-Film, DG11's build

**An unofficial build of [M-Film](https://github.com/HNGSeries/mfilm-canon-film-simulation) for two Canon cameras:**

| Camera | Firmware | Where |
|---|---|---|
| EOS 6D Mark II | 1.1.1 | [Releases](../../releases/latest) — this branch (`6d2`) |
| EOS 5D Mark III | 1.2.3 | [Release 0.2.4-5d3.1](../../releases/tag/v0.2.4-5d3.1) — the [`5d3-rainbow-fix`](../../tree/5d3-rainbow-fix) branch |

> [!NOTE]
> This is not the author's release. It is my own build of HNGSeries' M-Film, made and maintained by
> DG11 (davidgorun11). For the original, and for the cameras it supports, go to
> [HNGSeries/mfilm-canon-film-simulation](https://github.com/HNGSeries/mfilm-canon-film-simulation).
> Problems with this build belong [here](../../issues), not with the author.
>
> **Every release here was tested on a real camera before it was published.**

M-Film turns the camera into a film camera. Load a roll from a shelf of films, shoot it, then develop it
in the camera's darkroom. The original M-Film by HNGSeries runs on Canon's DIGIC 4 and 5 cameras. This build
makes it run on the 6D Mark II, a DIGIC 7 body that the original doesn't support, and makes it faster there.
On the 5D Mark III, which the original supports, it fixes the screen (no rainbow or Canon's menu over the
film OS on DIGIC 5), reads RAWs faster, develops the new frames at a press of RATE and keeps the original's
cleaner grain.

[![Download for 6D Mark II](https://img.shields.io/badge/Download%20for%206D%20Mark%20II-eca834?style=for-the-badge)](../../releases/tag/v0.2.1-6d2.3)
[![Download for 5D Mark III](https://img.shields.io/badge/Download%20for%205D%20Mark%20III-eca834?style=for-the-badge)](../../releases/tag/v0.2.4-5d3.1)
[![License GPL-2.0](https://img.shields.io/badge/license-GPL--2.0-3b352f)](LICENSE)

<img src="docs/images/6d2-shelf.png" width="720" alt="The film shelf on the 6D Mark II">

> [!WARNING]
> **Each zip is for one camera and one firmware only**, as its name says. Don't put it on any other camera.
> Magic Lantern on the 6D Mark II is experimental. On either camera you use it at your own risk.

## Install

Each zip has everything, Magic Lantern included: you don't install Magic Lantern separately.

### 6D Mark II

1. Check the camera's firmware: MENU → the last settings tab → *Firmware ver.* It must say 1.1.1.
2. Copy your photos off the card, then format the card in the camera.
3. Unzip `M-Film-0.2.1-6d2.3-camera-6D2.111.zip` onto the card's root. The top of the card must show
   `autoexec.bin`, `ML-SETUP.FIR` and the `ML` folder.
4. MENU → *Firmware ver.* → update. Follow the screen, then switch the camera off and on.
   This also marks the card to start Magic Lantern, and formatting the card in the camera clears that mark.
   Do this step again after every format.
5. Press **Trash** to open M-Film. This works with or without LiveView.

To update, format the card in the camera, copy the new zip's files to it and run step 4 again.
To remove M-Film, format the card: a card without Magic Lantern starts the camera as usual.

If the camera doesn't start, take the battery out and remove the card. Without the card it starts as usual.
`M-FILM-README.txt` in the zip has the details.

### 5D Mark III

1. Check the camera's firmware: MENU → the last settings tab → *Firmware ver.* It must say 1.2.3.
   A newer Canon firmware (1.3.x) must be taken back to 1.2.3 first: Magic Lantern doesn't run on it.
2. Copy your photos off the card, then format the card in the camera.
3. Unzip `M-Film-0.2.4-5d3.1-camera-5D3.123.zip` onto the card's root. The top of the card must show
   `autoexec.bin`, `ML-SETUP.FIR` and the `ML` folder.
4. MENU → *Firmware ver.* → update. Follow the screen, then switch the camera off and on.
5. Press **Trash** to open M-Film. In Canon's menu set Image quality to RAW and Image review to Off.

To update, develop the roll, copy the photos off, format the card in the camera and install again (steps 2 to 4).
`M-FILM-DG11-README.txt` in the zip lists what this build changes on the 5D Mark III.

## What's new

**0.2.4-5d3.1, the first for the 5D Mark III:** the author's M-Film 0.2.4 with a clean screen on entering and
leaving, LiveView off while M-Film is open, RAWs read faster, RATE to develop the roll's new frames (the newest
first) and the print right after, the rear dial to turn the prints and the top dial to zoom, PLAY for the prints
(from Canon's menus too) and Zoom for Canon's playback, a manual lens' aperture on the dial through the
viewfinder, and *About → INFO*: a QR code to this page. The grain stays as in 0.2.4: the author's newer grain
was too noisy on this camera.

**0.2.1-6d2.3:** the colour of the original M-Film. Each frame is developed with the white balance the camera
was on, the Film-Luts films get a finished photo's tone, and the glow round highlights is fainter and no
longer reddens the highlight itself. On the 68 films the two share, a print is pixel for pixel the original's.
The four cinema print films stay on the shelf here. No donation page or Darkrooms adverts in the camera.

**0.2.1-6d2.2:** a roll develops a third faster (both processor cores, 180 s → 117 s for 5 Large frames), and
M-Film opens and closes at once with no flash of Canon's last photo.

All changes: [CHANGELOG.md](CHANGELOG.md).

## What this fork adds

| | |
|---|---|
| **Runs on the 6D Mark II** | Magic Lantern changes for the 6D Mark II's DIGIC 7 and screen. The film OS is built as a module (`filmos.mo`) for its Cortex-A9 processor. |
| **Faster developing** | One frame takes about 10 s in Small and 21–25 s in Large. Before, Small took 26–29 s and Large 75–79 s. RAW reading went from 11.5 s to 1.6 s, the big prints use both of the DIGIC 7's cores, and LiveView is off while a roll develops. |
| **Less memory at power on** | 380 KB instead of 549 KB, because the film OS loads as a module after start-up. |
| **Print sizes and date stamp** | Small, Medium and Large prints, zoom in Prints (a setting, off by default), and a date stamp. These are ported from M-Film 0.2. |
| **Quick to open and close** | No shutter animation, and the screen stays black in between: Canon's last photo doesn't flash. The boot animation is a setting, off by default. |
| **Without LiveView** | M-Film opens with the viewfinder too, not only in LiveView. |
| **No readout over LiveView** | While you shoot you see Canon's own screen. The original's HUD and top plate are off on this camera: Canon's screen can't be kept from drawing over them on the 6D Mark II, and the two flickered over each other. The Viewfinder page and the wind lever went with them. |
| **One button scheme** | MENU goes back, PLAY opens Prints and Trash returns to the camera. INFO is the screen's second action and SET selects. This is the same on every screen. |
| **Fixes** | Dates in the roll list. No leftovers on screen after Prints. Trash works again in Canon's format screen, for low-level format. The shelf is cleaner on the 6D Mark II's screen. |

The technical notes are in [ml-dev/fork/ml-patches/6D2.111](ml-dev/fork/ml-patches/6D2.111/README.md).

## Buttons

| Button | What it does, on every screen |
|---|---|
| Trash | opens M-Film; inside it, back to the camera (in the Darkroom list it deletes the marked rolls) |
| MENU | back |
| PLAY | Prints (on the 6D Mark II's shooting screen: Canon's playback, unless *PLAY shows prints only* is on) |
| RATE (5D Mark III) | on the shooting screen: develops the roll's new frames now |
| Zoom (5D Mark III) | through the viewfinder: Canon's playback |
| Dials (5D Mark III) | in Prints: the rear one turns the frames, the top one zooms in |
| INFO | the screen's second action, shown at the bottom |
| SET | select, confirm |
| Half-press, LV | back to shooting |

## Build it yourself

```
docker build -t mfilm-build ml-dev/fork/ml-patches/6D2.111
docker run --rm -v "$PWD:/w" -w /w mfilm-build bash ml-dev/build_fork.sh 6D2.111
```

The card files end up in `ml-dev/out/fork-6D2.111/`. For the 5D Mark III, check out `5d3-rainbow-fix` and
build `5D3.123` against a Magic Lantern tree for it (`-e ML_SRC=...`); `ml-dev/build_fork.sh` says how.

## Problems

Report them in this fork's [Issues](../../issues), not to the original M-Film. Attach `ML/FILMROLL.LOG` from the card.

## Credits and license

- **M-Film** is by **HNGSeries**: the film OS, the 72 films and the darkroom. The original, for the other cameras, is at
  [HNGSeries/mfilm-canon-film-simulation](https://github.com/HNGSeries/mfilm-canon-film-simulation).
- It is built on [Magic Lantern](https://www.magiclantern.fm/) and its
  [magiclantern_simplified](https://github.com/reticulatedpines/magiclantern_simplified) port for newer cameras.
- The colours of 59 films are adapted from film emulation tables by [Pat David](https://patdavid.net) and are
  [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/), not GPL-2.0 (taken through G'MIC and
  [Film-Luts](https://github.com/YahiaAngelo/Film-Luts)); the 4 cinema print films use Juan Melara's tables. Those, the
  fonts and other parts: [THIRD_PARTY.md](THIRD_PARTY.md).
- License: GPL-2.0 ([LICENSE](LICENSE)).
- Not affiliated with Canon, Kodak, Fujifilm or any film maker.
