# M-Film on the Canon EOS 6D Mark II

**Firmware 1.1.1 · [Download the latest release](https://github.com/davidgorun11/m-film/releases/tag/v0.2.1-6d2.3) · DG11's build, not the author's release**

Back to the [main page](../../README.md).

The original M-Film doesn't support the 6D Mark II, a DIGIC 7 body. This build makes it run there, and faster.

<img src="../../docs/images/6d2-shelf.png" width="720" alt="The film shelf on the 6D Mark II">

> [!WARNING]
> The zip is for the 6D Mark II on firmware 1.1.1 only. Magic Lantern on the 6D Mark II is experimental.
> You use it at your own risk.

## Install

The zip has everything, Magic Lantern included: you don't install Magic Lantern separately.

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

## What's new

**0.2.1-6d2.3:** the colour of the original M-Film. Each frame is developed with the white balance the camera
was on, the Film-Luts films get a finished photo's tone, and the glow round highlights is fainter and no
longer reddens the highlight itself. On the 68 films the two share, a print is pixel for pixel the original's.
The four cinema print films stay on the shelf here. No donation page or Darkrooms adverts in the camera.

**0.2.1-6d2.2:** a roll develops a third faster (both processor cores, 180 s → 117 s for 5 Large frames), and
M-Film opens and closes at once with no flash of Canon's last photo.

All changes: [CHANGELOG.md](../../CHANGELOG.md).

## What this build adds

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

The technical notes are in [ml-dev/fork/ml-patches/6D2.111](../../ml-dev/fork/ml-patches/6D2.111/README.md).

## Buttons

| Button | What it does, on every screen |
|---|---|
| Trash | opens M-Film; inside it, back to the camera (in the Darkroom list it deletes the marked rolls) |
| MENU | back |
| PLAY | Prints (on the shooting screen: Canon's playback, unless *PLAY shows prints only* is on) |
| INFO | the screen's second action, shown at the bottom |
| SET | select, confirm |
| Half-press, LV | back to shooting |

## Build it yourself

From the `6d2` branch:

```
docker build -t mfilm-build ml-dev/fork/ml-patches/6D2.111
docker run --rm -v "$PWD:/w" -w /w mfilm-build bash ml-dev/build_fork.sh 6D2.111
```

The card files end up in `ml-dev/out/fork-6D2.111/`.
