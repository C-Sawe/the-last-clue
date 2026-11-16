# The Last Clue: meme edition (`memes` branch)

A fun version of the game with sound and meme moments. `main` stays the plain
submitted game; this branch adds the following.

| Where | What happens |
|---|---|
| Study | The main painting shows **mem1**; two new paintings (**mem3**, **mem2**) are just decoration |
| Painting close-up | The painting that swings aside is **mem1** |
| Torn letter | Stays on screen 4 s longer (about 6 s) before going back to the study; click the drawer again to re-read it |
| Safe | Click it again any time to look inside; the open safe holds the **GTA 6** case next to the key |
| Security panel | Simpler: the blue wire is already routed, only the green line's end (`255 75` → `255 225`) is wrong |
| Door opens | Creak, the lights flicker, then a **jump scare drawn in code**: a face built from our own primitives lunges at the camera (scaled 0.35× → 1.6× in 0.12 s), with a red flash, shake, glitch slices, static and a synthesised scream |
| Deduction (first time) | **mem4** ("let me think") fades in for 3.5 s |
| First wrong accusation | **lost1**, then **dofido1** with a GTA-style **WASTED – you let the murderer run away** |
| Case failed (3rd wrong) | **dofido1** + WASTED again |
| Case solved | **songmem1**, then **yaah!** the moment the CASE SOLVED stamp lands |
| Everywhere | Sound effects made in code: clicks, typing, dial ticks, drawer, door creak, safe, evidence chime, success, wrong buzz, whoosh on screen changes, the verdict stamp |

All memes are drawn **faded** (washed out, desaturated). **Click, Space, Enter or
Esc skips a meme.**

## Run it (macOS)

```bash
cd ~/the-last-clue
git fetch origin && git switch memes
cmake -B build -S . && cmake --build build -j
./build/the_last_clue          # run from the repository folder so it finds assets/memes
```

No extra installs: pictures are decoded with `stb_image` and sound is played with
`miniaudio` (both single files in `third_party/`, public domain).

## Changing the memes

Put the originals in a folder with the names above (`mem1.jpg`, `lost1.mp4`,
`yaah.mp3`, `gta6.png`, `jumpscare.mp4`, ...) and run (needs `brew install ffmpeg`):

```bash
bash tools/prepare_memes.sh ~/path/to/folder
```

It turns pictures into `assets/memes/<name>.jpg`, and each video into a folder of
JPEG frames (12 per second) plus `audio.mp3`. The game plays the frames itself,
scaled to the screen by inverse mapping, while miniaudio plays the sound. A
missing file just means that moment is skipped.

## How it is built

- `src/media/` – loads pictures and video frames, plays sounds, and
  **synthesises the sound effects** sample by sample (sine / square / saw
  oscillators, noise, decay envelopes, a one-pole low-pass filter).
- `src/game/memes.*` – the meme layer: a queue of clips drawn over the game view
  with the faded look, a shake + flash for the jump scare, and the WASTED effect
  (the frame drains to grey, the word drops in with OutBack easing).
- The game pauses while a meme plays. The scripted tests turn memes off
  (`Game::setMemes(false)`).
