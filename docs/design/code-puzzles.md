# Design: Code puzzles (writing code to open locked areas)

**Status:** planned. This is a design only, with no code yet. Implementation is in CP3 (interpreter, editor, first puzzle) and the final project (remaining puzzles). See the issues labelled [`code-puzzle`](https://github.com/C-Sawe/the-last-clue/issues?q=label%3Acode-puzzle).

## 1. The idea

Some locked parts of the crime scene can only be opened by **writing a short program** on the victim's computer. The player types code in an in-game editor and runs it. If the program produces the right result, the lock opens.

What keeps this a **computer graphics** feature is that the player's code **draws**. The built-in commands (`line`, `circle`, `forward`, `turn` and so on) call **our own algorithms**: Bresenham, midpoint circle and ellipse, and the transformation matrices. The player is, in effect, programming our graphics engine, and the game checks the picture their program draws.

These count towards the 4–6 puzzles in the proposal: they replace or sit alongside the cipher and hidden-object ideas.

## 2. The puzzles

| # | Puzzle | Opens | What the player writes | How it is checked |
|---|---|---|---|---|
| A | **Security panel wiring** | The study door / back room | `wire` / `line` commands that connect each coloured terminal to its matching socket | Every required pair is connected, and no two wires cross (segment-intersection test) |
| B | **The safe's electronic lock** | The safe | A loop that draws the victim's symbol, a 5-point star: `repeat 5 { forward 120  turn 144 }` | The drawing is rasterised and compared to the target shape (pixel overlap / IoU ≥ 90%) |
| C | **Encrypted email** *(optional)* | The victim's email, which gives the motive | A loop applying a Caesar shift to recover the message | The decoded text equals the expected text |

Each puzzle starts with a **broken or incomplete program** left on the computer (for example a loop that turns 72° instead of 144°). Most players will **debug** existing code rather than write from nothing, which keeps the difficulty reasonable.

## 3. The language: "ClueScript"

It is deliberately tiny: just enough for loops and drawing.

```
# comments start with #
let size = 120
repeat 5 {
    forward size
    turn 144
}
color red
circle 200 150 40
line 10 10 300 10
if size > 100 { print "big" }
```

| Feature | Supported |
|---|---|
| Values | numbers, strings |
| Variables | `let name = expr`, then `name = expr` to reassign |
| Expressions | `+ - * / %`, comparisons, parentheses |
| Control | `repeat N { … }`, `if cond { … } else { … }`, `while cond { … }` |
| Drawing | `line x1 y1 x2 y2`, `circle x y r`, `ellipse x y rx ry`, `fill_poly x1 y1 x2 y2 …`, `color name` |
| Turtle | `forward d`, `turn deg`, `pen up` / `pen down`, `goto x y` |
| Puzzle helpers | `wire A B`, `print value`, `shift text n` |

## 4. How it works (pure C++, our own implementation)

```
editor text ──► Lexer ──► Parser ──► AST ──► Interpreter ──► draw commands ──► our algorithms ──► preview canvas ──► Validator ──► unlock
              (tokens)  (recursive    (tree)  (tree-walking,   (line, circle,     (Bresenham,         (clipped to the     (puzzle-specific)  (flag set,
                         descent)              step limit)      turtle moves…)     midpoint, Mat3)     terminal screen)                       door/safe opens)
```

1. **Lexer:** turns text into tokens (numbers, names, strings, symbols) and records line numbers for error messages.
2. **Parser:** a recursive-descent parser builds an abstract syntax tree (AST). A syntax error gives a friendly message such as "Line 3: expected '}'".
3. **Interpreter:** walks the AST.
   - **Safety:** a hard step limit (for example 10,000 statements) stops infinite loops. There is no file or system access; the program can only draw and print.
   - **Turtle state:** position plus heading. `turn` updates the heading, and `forward` computes the end point by **rotating** the vector (d, 0) by the heading. That is the CP1 rotation formula `x' = x·cosθ − y·sinθ`, `y' = x·sinθ + y·cosθ`.
4. **Rendering:** each draw command calls our CP1 functions (`drawLineBresenham`, `drawCircleMidpoint`, `drawEllipseMidpoint`, `scanlineFill`). The output is **clipped** to the computer screen's rectangle (Cohen–Sutherland for lines, Sutherland–Hodgman for polygons), so the player's drawing can't escape the monitor.
5. **Validation:**
   - Puzzle A: a segment-intersection test checks that no wires cross, and point-in-polygon checks that each wire ends inside the correct socket.
   - Puzzle B: rasterise the player's drawing and the target star into two masks, then compute **intersection over union**. An IoU of 0.9 or more passes, so small rounding differences don't matter.
   - Puzzle C: string comparison.
6. **Unlock:** a pass sets a flag (`solved:wiring`, `solved:safe-lock`). The game state machine then plays the door or safe **opening animation** (from CP2) and adds the evidence to the Case File.

## 5. The in-game editor (CP3)

- A computer monitor drawn in the crime scene, which becomes a close-up when clicked (`scaleAbout`).
- A monospace text grid using our 5×7 bitmap font, with a blinking cursor, typing, backspace, enter and arrow keys. Keyboard input comes from GLFW's char/key callbacks.
- A **RUN** button, a **preview pane** that shows the program's drawing (or an error with its line number), and a **RESET** button that restores the original broken code.
- Optional extras: the line containing an error is highlighted, and hints unlock after 3 failed runs.

## 6. Graphics techniques this feature exercises

| Technique | Where |
|---|---|
| Bresenham, midpoint circle/ellipse, scanline fill (CP1) | Drawing the player's program output |
| Rotation (CP1) | Turtle heading: `forward` rotates the step vector |
| Translation / scaling (CP1/CP2) | Fitting the preview into the monitor; zooming into the computer |
| Cohen–Sutherland / Sutherland–Hodgman (CP1) | Clipping output to the monitor screen |
| Point in polygon, segment intersection | Validating the wiring puzzle |
| Rasterisation + IoU comparison | Validating the drawing puzzle |
| Animation (CP2) | Door and safe opening after a correct program |
| Bitmap text (CP1 font, CP3 editor) | Code editor and error messages |

## 7. Scope control

- **Must have:** the interpreter, the editor, and **one** code puzzle (B, the safe's electronic lock) in the CP3 prototype.
- **Should have:** puzzle A (wiring) in the final project.
- **Could have:** puzzle C (encrypted email), syntax highlighting and hints.
- **Fallback:** if typing proves too hard to finish in OpenGL, the puzzle becomes "fix the broken line": the player picks the right value from 3 options, and the same interpreter runs it.

## 8. Who builds what

| Milestone | Owner | Work |
|---|---|---|
| CP3 | Caleb | ClueScript interpreter: lexer, parser, AST, evaluator, step limit, drawing bindings |
| CP3 | Latiffa | In-game code editor: text input, cursor, RUN/RESET, preview pane, error console |
| CP3 | Sudheysi | Puzzle validation (IoU, wire checks) + unlock flags in the state machine |
| Final | Alvin | The computer terminal, security panel and the locked areas they open (art) |
| Final | Sudheysi | Puzzles A and C (wiring, email), with broken starter code and hints |
