// In-game code editor and the victim's laptop — Latiffa (CP3, issue #61)
#include "code_editor.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "core/font.h"
#include "polygon/polygon.h"
#include "primitives/primitives.h"

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
LaptopScreen::LaptopScreen() {
    runBtn_ = {20, 440, 140, 40, "RUN"};
    resetBtn_ = {170, 440, 140, 40, "RESET"};
    reset();
}

void LaptopScreen::reset() {
    editor_.setText(game::starStarterCode());
    hasRun_ = solved_ = false;
    failures_ = 0;
    last_ = {};
    check_ = {};
}

void LaptopScreen::mouseDown(PointF p) {
    mouse_ = p;
    if (runBtn_.contains(p)) run();
    else if (resetBtn_.contains(p) && !solved_) { reset(); message_ = "CODE RESET TO THE ORIGINAL"; }
    else if (p.x >= 20 && p.x <= 460 && p.y >= 50 && p.y <= 425) editor_.click(p);
}

void LaptopScreen::key(input::Key k) {
    if (k == input::Key::F1) run();
    else editor_.key(k);
}

void LaptopScreen::type(unsigned cp) { editor_.type(cp); }

void LaptopScreen::run() {
    last_ = script::run(editor_.text(), game::puzzleRunOptions());
    check_ = game::checkStar(last_);
    hasRun_ = true;
    if (check_.pass && !solved_) {
        solved_ = true;
        message_ = "ALARM DISABLED - THE SAFE CAN BE OPENED";
    } else if (!check_.pass) {
        ++failures_;
    }
}

std::string LaptopScreen::takeMessage() {
    std::string m = message_;
    message_.clear();
    return m;
}

void LaptopScreen::render(Canvas& c) const {
    c.clear(Color{40, 42, 52});
    drawText(c, 20, 14, "VICTIM'S LAPTOP  -  ALARM_KEY.CLUE", Palette::White, 2);
    editor_.render(c, 20, 50, 440, 375, time_, hasRun_ && !last_.ok ? last_.errorLine : 0);

    // Preview monitor: the program's drawing, clipped to the screen.
    const double px = 480, py = 50;
    scanlineFill(c, makeRect(px - 8, py - 8, 316, 316), Color{18, 18, 22});
    scanlineFill(c, makeRect(px, py, 300, 300), solved_ ? Color{225, 245, 228} : Palette::White);
    if (hasRun_) script::renderDrawing(c, last_.drawing, translate(px, py), ClipRect{px, py, px + 299, py + 299}, 3);
    else drawText(c, static_cast<int>(px) + 70, static_cast<int>(py) + 145, "PRESS RUN TO DRAW", Palette::Grey);

    drawButton(c, runBtn_, true, runBtn_.contains(mouse_), Color{40, 140, 70});
    drawButton(c, resetBtn_, !solved_, resetBtn_.contains(mouse_), Color{150, 60, 50});

    // Console: errors in red, check result, hint after three failed runs.
    std::string console = "THE ALARM WANTS HER SYMBOL. EDIT THE CODE, THEN RUN.";
    Color col = Color{180, 184, 200};
    if (hasRun_) {
        console = last_.ok ? check_.message : last_.error;
        col = check_.pass ? Color{120, 230, 140} : Color{240, 110, 100};
    }
    textBox(c, 480, 368, 300, 40, console, col);
    if (failures_ >= 3 && !solved_) textBox(c, 480, 400, 300, 60, game::starHint(), Color{255, 214, 60});
    drawText(c, 330, 452, "F1 = RUN", Color{120, 124, 140});
}

}  // namespace ui
