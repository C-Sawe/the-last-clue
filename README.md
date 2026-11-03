# The Last Clue

An interactive 2D detective puzzle game built for our Computer Graphics semester project. The player investigates a crime scene, solves graphical puzzles (a safe dial, a torn letter, an evidence board) and makes a final deduction: **who** did it, **how** and **why**. The graphics are the investigation tools: rotating the dial, moving and rotating torn pieces, and drawing strings between clues are all graphics operations.

Some locked areas, such as the safe's electronic lock and the security panel by the door, open only when the player **writes or fixes a short program** on the victim's computer. The game has its own tiny language, ClueScript, and an in-game code editor. The player's code **draws using our own algorithms** (Bresenham, midpoint circle and ellipse, rotation), and the game checks the drawing to decide whether the lock opens. This is planned for CP3 and the final project; see [`docs/design/code-puzzles.md`](docs/design/code-puzzles.md).

**Checkpoint 1 status:** every algorithm covered so far is implemented in **pure C++** and applied to components of the game. Each shape is produced by our own code writing individual pixels to a `Canvas`. No graphics library is used.

![Game layout](docs/screenshots/16_full_layout.png)

## Group members and responsibilities

| Member | GitHub | Role | Checkpoint 1 work |
|---|---|---|---|
| Caleb Kipchirchir | [@C-Sawe](https://github.com/C-Sawe) | Graphics & Algorithms | Canvas + BMP writer, bitmap font, DDA & Bresenham lines, midpoint circle & ellipse, 2D transformation matrices, integration |
| Alvin Kimani | [@alvviinn](https://github.com/alvviinn) | Environment & Visual Design | Polygon representation, scanline fill (edge table / active edge table), flood & boundary fill, crime scene, character line-up |
| Sudheysi Ali | [@Satasdin](https://github.com/Satasdin) | Puzzles & Game Mechanics | Cohen–Sutherland line clipping, Sutherland–Hodgman polygon clipping, point-in-polygon, safe dial, torn document puzzle, clipping/magnifier demo |
| Latiffa | [@latifahjaafar](https://github.com/latifahjaafar) | Interface, Animation & Integration | Bézier curves (de Casteljau), Catmull-Rom splines, evidence board, Case File panel, documentation and submission |

## Technologies and tools

- **C++17**: all graphics code, with no graphics library
- **CMake 3.10+**: build system (works with g++, clang++ and MSVC)
- **Git & GitHub**: branches per member, issues, milestones and pull requests
- **Output:** 24-bit BMP images written by our own encoder (`src/core/canvas.cpp`)
- **Later checkpoints:** OpenGL (CP3), once it is introduced in class

## Graphics techniques implemented so far

| Technique | Algorithm | Where it is used in the game | Code | Status |
|---|---|---|---|---|
| Line drawing | DDA, Bresenham (all 8 octants), thick lines | Evidence strings, wires, every outline | `src/primitives/line.cpp` | ✅ |
| Circles | Midpoint circle, 8-way symmetry, filled spans | Safe dial, board pins, clock, buttons | `src/primitives/circle.cpp` | ✅ |
| Ellipses | Midpoint ellipse (regions 1 and 2), filled | Heads, rug, shoes, fingerprint ridges | `src/primitives/ellipse.cpp` | ✅ |
| Polygons | Ordered vertex list, rectangles, regular polygons, stars | Furniture, coats, torn paper pieces | `src/polygon/polygon.cpp` | ✅ |
| Polygon filling | Scanline fill with edge table + active edge table | All filled shapes (convex and concave) | `src/polygon/fill.cpp` | ✅ |
| Seed filling | Flood fill and boundary fill (4-connected, explicit stack) | Region colouring | `src/polygon/fill.cpp` | ✅ |
| Line clipping | Cohen–Sutherland (outcodes) | Strings clipped to the viewport | `src/polygon/clip.cpp` | ✅ |
| Polygon clipping | Sutherland–Hodgman | Furniture clipped to the viewport | `src/polygon/clip.cpp` | ✅ |
| Hit testing | Point in polygon (ray casting, even-odd) | Torn pieces; mouse picking later | `src/polygon/clip.cpp` | ✅ |
| Bézier curves | de Casteljau (any degree) | Sagging evidence strings | `src/curves/bezier.cpp` | ✅ |
| Splines | Catmull-Rom (passes through every point) | Handwriting and signatures | `src/curves/spline.cpp` | ✅ |
| 2D transforms | 3×3 homogeneous matrices: translate, rotate/scale about a pivot, composite, inverse | Safe dial, document pieces, arms, magnifier | `src/transform/transform2d.cpp` | ✅ (early start on CP2) |

## Screenshots

| | |
|---|---|
| ![Crime scene](docs/screenshots/10_crime_scene.png) **Crime scene:** polygons, fills, circles, ellipses | ![Characters](docs/screenshots/11_characters.png) **Characters:** ellipses, rotated polygons, circles |
| ![Evidence board](docs/screenshots/15_evidence_board.png) **Evidence board:** Bézier strings, Catmull-Rom handwriting | ![Torn document](docs/screenshots/13_torn_document.png) **Torn letter:** polygon pieces + inverse transform mapping |
| ![Safe dial](docs/screenshots/12_safe_dial.png) **Safe dial:** circles + rotation about a pivot | ![Clipping](docs/screenshots/14_clipping_scene.png) **Clipping:** viewport and magnifier |

The algorithm test sheets (`01`–`09`) are in [`docs/screenshots/`](docs/screenshots), and every image is collected with captions in [`docs/checkpoint1_screenshots.pdf`](docs/checkpoint1_screenshots.pdf).

## Project structure

```
src/core/        Canvas (pixel buffer), Color/Point types, BMP writer, 5x7 bitmap font
src/primitives/  lines, circles, ellipses
src/polygon/     polygon representation, filling, clipping, point-in-polygon
src/curves/      Bézier and Catmull-Rom
src/transform/   2D transformation matrices
src/scenes/      game components built from the algorithms above
demos/           one program per demo; each writes an image to output/
scripts/         render_all.sh, GitHub setup scripts
docs/            screenshots, checkpoint PDFs, design documents (docs/design/)
```

## How to compile and run

You need CMake and a C++17 compiler (g++/MinGW, clang/Xcode tools or Visual Studio).

```bash
git clone https://github.com/C-Sawe/the-last-clue.git
cd the-last-clue
bash scripts/render_all.sh        # builds everything and runs every demo
```

Or build and run a single demo:

```bash
cmake -B build -S .
cmake --build build
./build/10_crime_scene             # Windows (Visual Studio): build\Debug\10_crime_scene.exe
```

Run demos from the **repository root**. The images appear in `output/` and open in any image viewer.

| Demo | Shows |
|---|---|
| `01_lines` … `09_transforms` | One test sheet per algorithm |
| `10_crime_scene`, `11_characters` | Environment and characters |
| `12_safe_dial`, `13_torn_document`, `14_clipping_scene` | Puzzles and clipping |
| `15_evidence_board`, `16_full_layout` | Evidence board and the full game window |

## Current progress

- [x] **Checkpoint 1 (Technical Foundation):** all algorithms covered so far, applied to game components
- [ ] **Checkpoint 2 (Transformation & Animation, week 10):** animation timeline, walk cycle, dial and document animations, GIF output
- [ ] **Checkpoint 3 (OpenGL + Prototype, week 12):** window and input, mouse picking, playable investigation loop, ClueScript interpreter + in-game code editor + first code puzzle
- [ ] **Final project (week 13):** deduction, 4–6 puzzles (including code puzzles: security wiring, encrypted email), polish

Progress is tracked in [Issues](https://github.com/C-Sawe/the-last-clue/issues) and [Milestones](https://github.com/C-Sawe/the-last-clue/milestones).

## Planned feature: code puzzles

| Part | Technique | Milestone |
|---|---|---|
| ClueScript interpreter (lexer → recursive-descent parser → AST → tree-walking evaluator, step limit) | Pure C++ | CP3 |
| Player's drawing commands (`line`, `circle`, `forward`/`turn` turtle) | Bresenham, midpoint circle/ellipse, rotation of the heading vector | CP3 |
| Output clipped to the in-game monitor | Cohen–Sutherland, Sutherland–Hodgman | CP3 |
| Checking the answer | Rasterised IoU comparison; point-in-polygon + segment intersection for wiring | CP3 / Final |
| In-game code editor | Bitmap font, keyboard input, RUN/RESET, error console | CP3 |
| Lock opens | Door/safe opening animation (transforms over time) | CP2 → CP3 |

The full design is in [`docs/design/code-puzzles.md`](docs/design/code-puzzles.md), and the work is tracked under the [`code-puzzle`](https://github.com/C-Sawe/the-last-clue/issues?q=label%3Acode-puzzle) label.

## Known limitations

- Output is static images. There is no window, mouse input or animation yet; those come in CP2 and CP3.
- Text uses our own 5×7 bitmap font, so it is upper-case only and has no anti-aliasing.
- Lines, circles and curves are aliased (no anti-aliasing).
- Rotated text is not supported: the labels on the safe dial move with the dial, but the characters stay upright.
- BMP is the only output format, so screenshots were converted to PNG for this README.

## Contributing workflow

1. Work on **your own branch** (`caleb`, `alvin`, `sudheysi`, `latiffa`). Run `git pull origin main` before starting.
2. Commit small and often, referencing the issue: `git commit -m "Add scanline fill (#12)"`.
3. Open a pull request into `main` with `Closes #N`, and ask a teammate to review it.
4. Make sure `git config user.email` matches your GitHub email, so that your commits count toward you.
