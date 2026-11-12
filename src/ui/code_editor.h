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

// Final build (issues #61 #62): the terminal runs ANY code puzzle. It holds
// one or more "files" (tabs), each a game::CodePuzzle with its own starter
// code, checker, hint and preview style (a drawing, a drawing over a
// background such as the wiring board, or printed text). Tabs can start
// locked and be unlocked by the game (the email only opens once the alarm is
// off). The victim's laptop and the security panel are both terminals.
class LaptopScreen {
public:
    LaptopScreen();                                // the victim's laptop (ALARM_KEY.CLUE + MAIL.CLUE)
    LaptopScreen(std::string title, std::vector<game::CodePuzzle> files);
    void reset();                                  // current file back to its starter code
    void mouseDown(PointF p);
    void mouseMove(PointF p) { mouse_ = p; }
    void key(input::Key k);
    void type(unsigned codepoint);
    void update(double dt) { time_ += dt; }
    void render(Canvas& c) const;                  // 800 x 500
    void run();                                    // RUN button / F1

    // Files (tabs)
    int fileCount() const { return static_cast<int>(files_.size()); }
    int current() const { return current_; }
    void open(int file);                           // switch tab (ignored if locked)
    void setLocked(int file, bool locked, const std::string& why = "");
    bool locked(int file) const { return files_[static_cast<size_t>(file)].locked; }
    bool solved(int file) const { return files_[static_cast<size_t>(file)].solved; }
    bool solved() const { return solved(0); }      // CP3: the first file

    std::string takeMessage();
    CodeEditor& editor() { return editor_; }

private:
    struct File {
        game::CodePuzzle puzzle;
        std::string text;                          // the code while another tab is open
        script::RunResult last;
        game::CodeCheck check;
        bool hasRun = false, solved = false, locked = false;
        int failures = 0;
        std::string lockedWhy;
    };
    std::string title_;
    std::vector<File> files_;
    int current_ = 0;
    CodeEditor editor_;
    double time_ = 0;
    PointF mouse_{0, 0};
    std::string message_;
    Button runBtn_, resetBtn_;
    Button tab(int i) const;
    File& cur() { return files_[static_cast<size_t>(current_)]; }
    const File& cur() const { return files_[static_cast<size_t>(current_)]; }
};

}  // namespace ui
