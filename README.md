# The Last Clue

An interactive 2D detective puzzle game built for our Computer Graphics semester project. The player investigates a crime scene, solves graphical puzzles (safe dial, torn document, evidence board), and makes a final deduction: **who**, **how** and **why**.

All graphics in Checkpoint 1 are drawn with **pure C++**. Every shape is produced by our own algorithms writing individual pixels to a `Canvas`. No graphics library is used for drawing.

## Group members and responsibilities

| Member | Role | Checkpoint 1 ownership |
|---|---|---|
| Caleb Kipchirchir | Graphics & Algorithms | Canvas/BMP core, DDA & Bresenham lines, midpoint circle & ellipse, 2D transforms, PR reviews |
| Alvin Kimani | Environment & Visual Design | Polygon representation, scanline & flood fill, crime scene, character line-up |
| Sudheysi Ali | Puzzles & Game Mechanics | Cohen–Sutherland & Sutherland–Hodgman clipping, point-in-polygon, safe dial, torn document, clipping demo |
| Latiffa | Interface, Animation & Integration | Bézier & Catmull-Rom curves, evidence board, Case File panel, README, screenshot PDF, form submission |

## Technologies and tools

- C++17
- CMake 3.10+ (or any C++17 compiler: g++, clang++, MSVC)
- Git & GitHub (issues, milestones, pull requests)
- Output format: 24-bit BMP written by our own encoder

## Graphics techniques and algorithms

| Technique | Algorithm | Used for | Status |
|---|---|---|---|
| Lines | DDA, Bresenham | Evidence-board strings, wires, outlines | ⬜ |
| Circles | Midpoint circle | Safe dial, buttons, pins, magnifier | ⬜ |
| Ellipses | Midpoint ellipse | Character heads/bodies, rug, fingerprints | ⬜ |
| Polygons | Vertex-list representation | Furniture, characters, torn paper pieces | ⬜ |
| Filling | Scanline fill, flood/boundary fill | Furniture, paper, suspect colour-coding | ⬜ |
| Clipping | Cohen–Sutherland, Sutherland–Hodgman | Scene viewport, magnifier window | ⬜ |
| Curves | Bézier (de Casteljau), Catmull-Rom | Sagging strings, handwriting | ⬜ |
| Transforms | Translate / rotate / scale (3×3 matrices) | Safe dial, document pieces | ⬜ |

*(Update ⬜ → ✅ as each issue is closed.)*

## Project structure

```
src/core/        Canvas, Color, Point, BMP writer
src/primitives/  lines, circles, ellipses
src/polygon/     polygon representation, filling, clipping
src/curves/      Bézier and splines
src/transform/   2D transformation matrices
src/scenes/      game components built from the algorithms above
demos/           one executable per demo; each writes an image to output/
output/          rendered screenshots (committed as evidence)
docs/            wireframes, checkpoint PDFs
```

## How to compile and run

```bash
cmake -B build -S .
cmake --build build
./build/canvas_test          # Windows: build\Debug\canvas_test.exe
```

Run demos from the **repository root**, so that images land in `output/`. Any new `.cpp` file in `src/` or `demos/` is picked up automatically after you re-run `cmake -B build -S .`.

## Current progress

- [x] Repository, build system, `Canvas` and BMP writer
- [ ] Algorithms (Milestone: *Checkpoint 1 – Algorithms*)
- [ ] Project scenes (Milestone: *Checkpoint 1 – Project Components*)
- [ ] Screenshots PDF and submission

## Known limitations

- Output is currently static images; there is no window or mouse input yet. Interaction is planned for weeks 7–8.
- Only BMP output is supported. Convert to PNG for the report if needed.

## Contributing workflow

1. Work on **your own branch** (`caleb`, `alvin`, `sudheysi`, `latiffa`). Start each session with `git pull origin main`.
2. Commit small and often, and reference the issue: `git commit -m "Add Bresenham for steep lines (#3)"`.
3. Open a pull request into `main` and ask one teammate to review. Use `Closes #N` in the PR to close the issue.
4. Check `git config user.email` matches your GitHub email, so that your commits count toward you.
