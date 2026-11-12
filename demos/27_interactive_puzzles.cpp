// Demo (CP3, issues #41/#43): the interactive puzzles driven by SCRIPTED mouse
// input — the same calls the game window makes when a player drags and scrolls.
//   1. Torn letter: grab each piece, drag it home, scroll to rotate until it snaps.
//   2. Safe: drag around the dial to 37, ENTER, 12, ENTER, 85, ENTER -> door opens.
// Output: output/27_interactive_puzzles.gif / .bmp (key frames)
#include <cmath>
#include <iostream>

#include "demo_common.h"
#include "game/puzzles.h"

namespace {
const double PI = 3.14159265358979;

struct Recorder {
    GifWriter gif{800, 500, 5};
    std::vector<Canvas> keys;
    void frame(const Canvas& c, int hold = 1) { gif.addFrame(c, hold); }
};

void cursor(Canvas& c, PointF p) {                 // draw the "mouse" so the GIF shows it
    const int x = static_cast<int>(p.x), y = static_cast<int>(p.y);
    for (int i = 0; i < 12; ++i) for (int j = 0; j <= i / 2; ++j) { c.setPixel(x + j, y + i, Palette::White); }
    for (int i = 0; i < 12; ++i) c.setPixel(x, y + i, Palette::Black);
}
}  // namespace

int main() {
    Recorder rec;
    Canvas c(800, 500);

    // ---------------- Letter puzzle ----------------
    game::LetterPuzzle letter;
    auto showLetter = [&](PointF m, int hold = 1) { letter.render(c); cursor(c, m); rec.frame(c, hold); };
    showLetter({400, 250}, 8);
    rec.keys.push_back(c);
    for (int i = 0; i < 4; ++i) {
        const PointF from = letter.pieceScreenCentre(i), to = letter.solvedScreenCentre(i);
        letter.mouseMove(from);
        letter.mouseDown(from);
        for (int s = 1; s <= 10; ++s) {                                      // drag in 10 steps
            const PointF m{from.x + (to.x - from.x) * s / 10.0, from.y + (to.y - from.y) * s / 10.0};
            letter.mouseMove(m);
            showLetter(m);
        }
        letter.mouseUp(to);
        const int steps = static_cast<int>(std::lround(std::fabs(letter.pieceAngle(i)) / (PI / 12)));
        const double dir = letter.pieceAngle(i) > 0 ? -1 : 1;
        for (int s = 0; s < steps; ++s) { letter.scroll(to, dir); showLetter(to, 2); }  // wheel notches
        if (i == 1) rec.keys.push_back(c);
    }
    showLetter({400, 470}, 25);
    rec.keys.push_back(c);
    std::cout << "Letter solved: " << (letter.solved() ? "yes" : "NO") << "\n";

    // ---------------- Safe puzzle ----------------
    game::SafePuzzle safe;
    const PointF ctr = safe.dialCentre();
    auto showSafe = [&](PointF m, int hold = 1) { safe.render(c); cursor(c, m); rec.frame(c, hold); };
    double mouseAngle = -PI / 2;                                              // grab the top of the dial
    auto mouseAt = [&](double a) { return PointF{ctr.x + 120 * std::cos(a), ctr.y + 120 * std::sin(a)}; };
    showSafe(mouseAt(mouseAngle), 6);
    for (int n : {37, 12, 85}) {
        const double current = -2 * PI * safe.number() / 100, target = -2 * PI * n / 100;
        double delta = std::remainder(target - current, 2 * PI);              // shortest way round
        safe.mouseDown(mouseAt(mouseAngle));
        const int steps = std::max(6, static_cast<int>(std::fabs(delta) / 0.12));
        for (int s = 1; s <= steps; ++s) {
            mouseAngle += delta / steps;
            safe.mouseMove(mouseAt(mouseAngle));
            showSafe(mouseAt(mouseAngle));
        }
        safe.mouseUp(mouseAt(mouseAngle));
        while (safe.number() != n) safe.key(safe.number() < n ? input::Key::Left : input::Key::Right);   // exact
        safe.key(input::Key::Enter);
        showSafe(mouseAt(mouseAngle), 8);
    }
    rec.keys.push_back(c);
    for (int f = 0; f < 30; ++f) { safe.update(1.0 / 20); showSafe(mouseAt(mouseAngle)); }
    showSafe(mouseAt(mouseAngle), 25);
    rec.keys.push_back(c);
    std::cout << "Safe solved: " << (safe.solved() && safe.doorOpen() ? "yes" : "NO") << "\n";

    if (!rec.gif.save("output/27_interactive_puzzles.gif")) return 1;
    std::cout << "Wrote output/27_interactive_puzzles.gif (" << rec.gif.frameCount() << " frames)\n";
    Canvas sheet(800, 750, Color{30, 32, 40});                               // 2 x 3 key frames
    for (size_t k = 0; k < rec.keys.size() && k < 6; ++k)
        for (int y = 0; y < 250; ++y)
            for (int x = 0; x < 400; ++x)
                sheet.setPixel(static_cast<int>(k % 2) * 400 + x, static_cast<int>(k / 2) * 250 + y, rec.keys[k].getPixel(x * 2, y * 2));
    return demo::save(sheet, "27_interactive_puzzles");
}
