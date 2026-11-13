// In-game code editor and the victim's laptop — Latiffa (CP3, issue #61)
#include "code_editor.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "core/font.h"
#include "polygon/polygon.h"
#include "primitives/primitives.h"
#include "scenes/scenes.h"

namespace ui {

// ================================ CodeEditor ================================
void CodeEditor::setText(const std::string& text) {
    lines_.clear();
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) lines_.push_back(l);
    if (lines_.empty()) lines_.push_back("");
    row_ = col_ = top_ = 0;
}

std::string CodeEditor::text() const {
    std::string out;
    for (size_t i = 0; i < lines_.size(); ++i) out += lines_[i] + (i + 1 < lines_.size() ? "\n" : "");
    return out;
}

void CodeEditor::clampCursor() {
    row_ = std::max(0, std::min(row_, static_cast<int>(lines_.size()) - 1));
    col_ = std::max(0, std::min(col_, static_cast<int>(lines_[static_cast<size_t>(row_)].size())));
}

void CodeEditor::type(unsigned cp) {
    if (cp < 32 || cp > 126) return;                       // our font is ASCII only
    std::string& l = lines_[static_cast<size_t>(row_)];
    l.insert(l.begin() + col_, static_cast<char>(cp));
    ++col_;
}

void CodeEditor::key(input::Key k) {
    std::string& l = lines_[static_cast<size_t>(row_)];
    switch (k) {
        case input::Key::Enter: {                           // split the line, keep indentation
            std::string rest = l.substr(static_cast<size_t>(col_));
            l.erase(static_cast<size_t>(col_));
            const size_t indent = l.find_first_not_of(' ') == std::string::npos ? l.size() : l.find_first_not_of(' ');
            const std::string pad(indent, ' ');
            lines_.insert(lines_.begin() + row_ + 1, pad + rest);
            ++row_;
            col_ = static_cast<int>(pad.size());
            break;
        }
        case input::Key::Backspace:
            if (col_ > 0) { l.erase(static_cast<size_t>(col_ - 1), 1); --col_; }
            else if (row_ > 0) {                            // join with the previous line
                col_ = static_cast<int>(lines_[static_cast<size_t>(row_ - 1)].size());
                lines_[static_cast<size_t>(row_ - 1)] += l;
                lines_.erase(lines_.begin() + row_);
                --row_;
            }
            break;
        case input::Key::Delete:
            if (col_ < static_cast<int>(l.size())) l.erase(static_cast<size_t>(col_), 1);
            else if (row_ + 1 < static_cast<int>(lines_.size())) {
                l += lines_[static_cast<size_t>(row_ + 1)];
                lines_.erase(lines_.begin() + row_ + 1);
            }
            break;
        case input::Key::Tab: lines_[static_cast<size_t>(row_)].insert(static_cast<size_t>(col_), "  "); col_ += 2; break;
        case input::Key::Left:
            if (col_ > 0) --col_;
            else if (row_ > 0) { --row_; col_ = static_cast<int>(lines_[static_cast<size_t>(row_)].size()); }
            break;
        case input::Key::Right:
            if (col_ < static_cast<int>(l.size())) ++col_;
            else if (row_ + 1 < static_cast<int>(lines_.size())) { ++row_; col_ = 0; }
            break;
        case input::Key::Up: --row_; break;
        case input::Key::Down: ++row_; break;
        case input::Key::Home: col_ = 0; break;
        case input::Key::End: col_ = static_cast<int>(l.size()); break;
        default: break;
    }
    clampCursor();
}

// Pixel -> (row, column): undo the grid arithmetic used when drawing.
void CodeEditor::click(PointF p) {
    row_ = top_ + static_cast<int>((p.y - boxY_ - 6) / lineH(Scale));
    col_ = static_cast<int>(std::lround((p.x - boxX_ - Gutter - 6) / charW(Scale)));
    clampCursor();
}

void CodeEditor::render(Canvas& c, double x, double y, double w, double h, double time, int errorLine) const {
    boxX_ = x; boxY_ = y; boxH_ = h;
    scanlineFill(c, makeRect(x, y, w, h), Color{22, 24, 30});
    scanlineFill(c, makeRect(x, y, Gutter, h), Color{32, 34, 42});
    drawPolygonOutline(c, makeRect(x, y, w, h), Color{70, 74, 90});
    const int visible = static_cast<int>((h - 12) / lineH(Scale));
    int top = top_;
    if (row_ < top) top = row_;
    if (row_ >= top + visible) top = row_ - visible + 1;
    top_ = top;                                               // keep the cursor on screen

    for (int i = 0; i < visible && top + i < static_cast<int>(lines_.size()); ++i) {
        const int r = top + i;
        const int ly = static_cast<int>(y) + 6 + i * lineH(Scale);
        if (r + 1 == errorLine) scanlineFill(c, makeRect(x + Gutter, ly - 2, w - Gutter - 1, lineH(Scale)), Color{90, 30, 34});
        drawText(c, static_cast<int>(x) + 4, ly + 2, std::to_string(r + 1), Color{110, 114, 130});
        drawText(c, static_cast<int>(x) + Gutter + 6, ly, lines_[static_cast<size_t>(r)],
                 lines_[static_cast<size_t>(r)].find('#') == 0 ? Color{120, 140, 170} : Color{150, 230, 160}, Scale);
    }
    // Blinking cursor: on for half a second, off for half a second.
    if (std::fmod(time, 1.0) < 0.5 && row_ >= top && row_ < top + visible) {
        const int cx = static_cast<int>(x) + Gutter + 6 + col_ * charW(Scale) - 1;
        const int cy = static_cast<int>(y) + 6 + (row_ - top) * lineH(Scale) - 1;
        drawThickLine(c, {cx, cy}, {cx, cy + 15}, 2, Palette::White);
    }
}

// =============================== LaptopScreen ===============================
LaptopScreen::LaptopScreen()
    : LaptopScreen("VICTIM'S LAPTOP", {game::starPuzzle(), game::emailPuzzle()}) {
    setLocked(1, true, "LOCKED BY THE ALARM - DISABLE IT FIRST");
}

LaptopScreen::LaptopScreen(std::string title, std::vector<game::CodePuzzle> files) : title_(std::move(title)) {
    runBtn_ = {20, 440, 140, 40, "RUN"};
    resetBtn_ = {170, 440, 140, 40, "RESET"};
    for (auto& p : files) {
        File f;
        f.puzzle = std::move(p);
        f.text = f.puzzle.starter;
        files_.push_back(std::move(f));
    }
    editor_.setText(files_.front().text);
}

void LaptopScreen::reset() {
    File& f = cur();
    f.text = f.puzzle.starter;
    f.hasRun = f.solved = false;
    f.failures = 0;
    f.last = {};
    f.check = {};
    editor_.setText(f.text);
}

void LaptopScreen::open(int file) {
    if (file < 0 || file >= fileCount() || file == current_) return;
    if (files_[static_cast<size_t>(file)].locked) { message_ = files_[static_cast<size_t>(file)].lockedWhy; return; }
    cur().text = editor_.text();                              // keep the edits of the tab we leave
    current_ = file;
    editor_.setText(cur().text);
}

void LaptopScreen::setLocked(int file, bool locked, const std::string& why) {
    File& f = files_[static_cast<size_t>(file)];
    f.locked = locked;
    if (!why.empty()) f.lockedWhy = why;
}

// Tabs sit in a row above the editor, one per file.
Button LaptopScreen::tab(int i) const { return {20.0 + i * 156, 34, 150, 18, files_[static_cast<size_t>(i)].puzzle.file}; }

void LaptopScreen::mouseDown(PointF p) {
    mouse_ = p;
    for (int i = 0; i < fileCount(); ++i)
        if (fileCount() > 1 && tab(i).contains(p)) { open(i); return; }
    if (runBtn_.contains(p)) run();
    else if (resetBtn_.contains(p) && !cur().solved) { reset(); message_ = "CODE RESET TO THE ORIGINAL"; }
    else if (p.x >= 20 && p.x <= 460 && p.y >= 58 && p.y <= 425) editor_.click(p);
}

void LaptopScreen::key(input::Key k) {
    if (k == input::Key::F1) run();
    else if (k == input::Key::Tab && fileCount() > 1) open((current_ + 1) % fileCount());
    else editor_.key(k);
}

void LaptopScreen::type(unsigned cp) { editor_.type(cp); }

void LaptopScreen::run() {
    File& f = cur();
    f.last = script::run(editor_.text(), f.puzzle.options);
    f.check = f.puzzle.check ? f.puzzle.check(f.last) : game::CodeCheck{};
    f.hasRun = true;
    if (f.check.pass && !f.solved) {
        f.solved = true;
        message_ = f.check.message;
    } else if (!f.check.pass) {
        ++f.failures;
    }
}

std::string LaptopScreen::takeMessage() {
    std::string m = message_;
    message_.clear();
    return m;
}

void LaptopScreen::render(Canvas& c) const {
    const File& f = cur();
    c.clear(Color{40, 42, 52});
    drawText(c, 20, 10, title_ + "  -  " + f.puzzle.file, Palette::White, 2);

    // Tabs: the open one joins the editor; locked ones are dimmed; solved ones get a tick colour.
    if (fileCount() > 1)
        for (int i = 0; i < fileCount(); ++i) {
            const Button b = tab(i);
            const File& t = files_[static_cast<size_t>(i)];
            const Color fill = i == current_ ? Color{24, 26, 32} : (b.contains(mouse_) && !t.locked ? Color{70, 74, 90} : Color{54, 57, 70});
            scanlineFill(c, makeRect(b.x, b.y, b.w, b.h + (i == current_ ? 4 : 0)), fill);
            drawPolygonOutline(c, makeRect(b.x, b.y, b.w, b.h), Color{70, 74, 90});
            const Color ink = t.locked ? Color{110, 112, 126} : (t.solved ? Feedback::Correct : Palette::White);
            drawText(c, static_cast<int>(b.x) + 6, static_cast<int>(b.y) + 6, t.locked ? b.label + " (LOCKED)" : b.label, ink);
        }
    editor_.render(c, 20, 56, 440, 369, time_, f.hasRun && !f.last.ok ? f.last.errorLine : 0);

    // Preview monitor: the program's drawing (over the puzzle's own background,
    // if it has one), clipped to the screen - or the printed text.
    const double px = 480, py = 56;
    scanlineFill(c, makeRect(px - 8, py - 8, 316, 316), Color{18, 18, 22});
    const ClipRect screen{px, py, px + 299, py + 299};
    if (f.puzzle.showsText) {
        scanlineFill(c, makeRect(px, py, 300, 300), Color{16, 22, 20});
        int y = static_cast<int>(py) + 10;
        if (!f.hasRun) {
            for (const auto& l : wrap(game::emailCipherText(), 23)) { drawText(c, static_cast<int>(px) + 10, y, l, Color{90, 140, 100}, 2); y += 20; }
            drawText(c, static_cast<int>(px) + 10, static_cast<int>(py) + 280, "SCRAMBLED - PRESS RUN", Palette::Grey);
        } else {
            for (const auto& line : f.last.printed)
                for (const auto& l : wrap(line, 23)) {
                    if (y > py + 280) break;
                    drawText(c, static_cast<int>(px) + 10, y, l, f.solved ? Feedback::Correct : Color{150, 230, 160}, 2);
                    y += 20;
                }
        }
    } else {
        scanlineFill(c, makeRect(px, py, 300, 300), f.solved ? Color{225, 245, 228} : Palette::White);
        if (f.puzzle.background) f.puzzle.background(c, translate(px, py), f.solved);
        if (f.hasRun) script::renderDrawing(c, f.last.drawing, translate(px, py), screen, f.puzzle.background ? 4 : 3);
        else if (!f.puzzle.background)
            drawText(c, static_cast<int>(px) + 70, static_cast<int>(py) + 145, "PRESS RUN TO DRAW", Palette::Grey);
    }

    drawButton(c, runBtn_, true, runBtn_.contains(mouse_), Color{40, 140, 70});
    drawButton(c, resetBtn_, !f.solved, resetBtn_.contains(mouse_), Color{150, 60, 50});

    // Console: goal before the first run, then errors (red) or the check result;
    // the hint appears after three failed runs.
    std::string console = f.puzzle.goal;
    Color col = Feedback::Info;
    if (f.hasRun) {
        console = f.last.ok ? f.check.message : f.last.error;
        col = f.check.pass ? Feedback::Correct : Feedback::Wrong;
    }
    textBox(c, 480, 368, 300, 40, console, col);
    if (f.failures >= 3 && !f.solved) textBox(c, 480, 400, 300, 60, f.puzzle.hint, Feedback::Hint);
    drawText(c, 330, 446, "F1 = RUN", Color{120, 124, 140});
    if (fileCount() > 1) drawText(c, 330, 462, "TAB = NEXT FILE", Color{120, 124, 140});
}

}  // namespace ui
