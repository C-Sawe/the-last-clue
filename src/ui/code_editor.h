#pragma once
// In-game code editor and the victim's laptop — Latiffa (CP3, issue #61)
//
// CodeEditor: a small text editor drawn with our bitmap font on a monospace
// grid. Because every character is exactly charW x lineH pixels, converting
// between (row, column) and screen pixels is simple arithmetic — that is how
// clicks place the cursor and how the cursor and error line are drawn.
//
// LaptopScreen: the editor + RUN / RESET buttons + a preview monitor showing
// what the program draws (our interpreter + our algorithms) + a console line.
// RUN checks the drawing with the code-puzzle validator (IoU).
#include <string>
#include <vector>

#include "core/keys.h"
#include "game/code_check.h"
#include "text.h"

namespace ui {

class CodeEditor {
public:
    void setText(const std::string& text);
    std::string text() const;

    void type(unsigned codepoint);                 // printable characters
    void key(input::Key k);                        // Enter, Backspace, arrows, ...
    void click(PointF p);                          // place the cursor (screen coords)

    // Draws inside the box; highlights `errorLine` (1-based, 0 = none).
    void render(Canvas& c, double x, double y, double w, double h, double time, int errorLine) const;

    int row() const { return row_; }
    int col() const { return col_; }

private:
    std::vector<std::string> lines_{""};
    int row_ = 0, col_ = 0;
    mutable int top_ = 0;                          // first visible line (scrolls to follow the cursor)
    mutable double boxX_ = 0, boxY_ = 0, boxH_ = 0;  // last drawn position (for clicks)
    static const int Scale = 2;
    static const int Gutter = 30;                  // line numbers

    void clampCursor();
};

class LaptopScreen {
public:
    LaptopScreen();
    void reset();

    void mouseDown(PointF p);
    void mouseMove(PointF p) { mouse_ = p; }
    void key(input::Key k);
    void type(unsigned codepoint);
    void update(double dt) { time_ += dt; }

    void render(Canvas& c) const;                  // 800 x 500
    void run();                                    // RUN button / F5-style action
    bool solved() const { return solved_; }
    std::string takeMessage();
    CodeEditor& editor() { return editor_; }

private:
    CodeEditor editor_;
    script::RunResult last_;
    game::CodeCheck check_;
    bool hasRun_ = false, solved_ = false;
    int failures_ = 0;
    double time_ = 0;
    PointF mouse_{0, 0};
    std::string message_;
    Button runBtn_, resetBtn_;
};

}  // namespace ui
