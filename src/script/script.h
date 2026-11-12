#pragma once
// ClueScript — the tiny language players use in the code puzzles.
// Caleb (CP3, issue #58). Design: docs/design/code-puzzles.md
//
// Pipeline:  source text --Lexer--> tokens --Parser--> AST --Interpreter--> drawing
//
//   # comments start with #
//   let size = 120
//   repeat 5 {
//       forward size
//       turn 144
//   }
//   color red
//   circle 200 150 40
//
// The program never touches the screen directly: it produces a list of draw
// commands. renderDrawing() then draws them with OUR algorithms (Bresenham,
// midpoint circle/ellipse, scanline fill), clipped to a window.
#include <string>
#include <vector>

#include "core/canvas.h"
#include "polygon/polygon.h"
#include "transform/transform2d.h"

namespace script {

struct DrawCmd {
    enum class Kind { Line, Circle, Ellipse, FillPoly } kind;
    std::vector<PointF> pts;    // Line: 2 points; Circle/Ellipse: centre; FillPoly: vertices
    double rx = 0, ry = 0;      // Circle: rx = radius; Ellipse: rx, ry
    Color color;
};

struct RunOptions {
    PointF start{150, 150};     // turtle start position (program coordinates)
    double heading = 0;         // degrees, 0 = facing right; turn is clockwise on screen
    int stepLimit = 10000;      // stops infinite loops
};

struct RunResult {
    bool ok = true;
    int errorLine = 0;          // 1-based line of the first error (0 = none)
    std::string error;          // friendly message, e.g. "Line 3: expected '}'"
    std::vector<DrawCmd> drawing;
    std::vector<std::string> printed;
    int steps = 0;
};

// Lex, parse and run a program. Never throws.
RunResult run(const std::string& source, const RunOptions& opt = RunOptions{});

// Draw a program's output with our algorithms. `toScreen` maps program
// coordinates to canvas coordinates; everything is clipped to `clip`.
void renderDrawing(Canvas& c, const std::vector<DrawCmd>& drawing, const Mat3& toScreen,
                   const ClipRect& clip, int thickness = 2);

}  // namespace script
