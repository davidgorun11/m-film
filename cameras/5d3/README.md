# M-Film on the Canon EOS 5D Mark III

**Firmware 1.2.3 · [Download release 0.2.4-5d3.1](https://github.com/davidgorun11/m-film/releases/tag/v0.2.4-5d3.1) · DG11's build, not the author's release**

Back to the [main page](../../README.md).

The original M-Film runs on the 5D Mark III. This build is the author's 0.2.4 with the screen fixed on DIGIC 5,
a faster darkroom and buttons that suit the camera. It was tested on a real 5D Mark III.

> [!WARNING]
> The zip is for the 5D Mark III on firmware 1.2.3 only. You use it at your own risk.

## Install

The zip has everything, Magic Lantern included: you don't install Magic Lantern separately.

1. Check the camera's firmware: MENU → the last settings tab → *Firmware ver.* It must say 1.2.3.
   A newer Canon firmware (1.3.x) must be taken back to 1.2.3 first: Magic Lantern doesn't run on it.
2. Copy your photos off the card, then format the card in the camera.
3. Unzip `M-Film-0.2.4-5d3.1-camera-5D3.123.zip` onto the card's root. The top of the card must show
   `autoexec.bin`, `ML-SETUP.FIR` and the `ML` folder.
4. MENU → *Firmware ver.* → update. Follow the screen, then switch the camera off and on.
5. Press **Trash** to open M-Film.

In Canon's menu set Image quality to RAW and Image review to Off.

To update, develop the roll, copy the photos off, format the card in the camera and install again (steps 2 to 4).
To remove M-Film, format the card: a card without Magic Lantern starts the camera as usual.

## What's different from the author's 0.2.4

| | |
|---|---|
| **The screen** | No rainbow, garbage or Canon's menu over M-Film on entering, leaving or starting the camera. LiveView goes off while M-Film is open. |
| **Darkroom** | RAWs read faster. **RATE** on the shooting screen develops the roll's new frames, the newest first, then shows the print; a press of the shutter that cuts it short is followed by another go. About 45 s a frame in Large with zoom on. Damaged RAWs are refused with a reason (from the author's main branch). |
| **Grain** | As in 0.2.4. The author's newer grain was tried and was too noisy on this camera's prints. |
| **Prints** | The rear dial turns the frames, the top dial zooms in and out, the joystick turns the frames or moves round the zoomed print. |
| **Shelf** | Left and right change the shelf, the dials go along the films. |
| **Settings** | Zoom in Prints (off by default, develops faster), Drive single frame, a manual lens' aperture on the dial through the viewfinder (Av: top dial, M: rear dial). No Canon menu row: MENU on the shooting screen opens it. *About → INFO*: a QR code to this repository. |
| **Films** | 72, the four cinema print films included. No donation pages. |

Magic Lantern in the zip is the author's, from M-Film 0.2.4, unchanged. Only the film OS module
(`ML/modules/filmos.mo`) and its films differ.

## Buttons

| Button | What it does |
|---|---|
| Trash | on the shooting screen: opens M-Film; inside it, back to the camera |
| RATE | on the shooting screen: develops the roll's new frames now |
| PLAY | the latest prints, from the shooting screen and from Canon's menus |
| Zoom | with LiveView off: Canon's playback |
| MENU | inside M-Film: back; on the shooting screen: Canon's menu |
| INFO | the screen's second action, shown at the bottom |
| SET | select, confirm |
| Rear dial, top dial | in Prints: turn the frames, zoom in |
| Half-press | back to shooting |

## Build it yourself

The code is in the [`5d3-rainbow-fix`](https://github.com/davidgorun11/m-film/tree/5d3-rainbow-fix) branch.
Build `5D3.123` with `ml-dev/build_fork.sh` against a Magic Lantern tree for the 5D Mark III (`-e ML_SRC=...`).
