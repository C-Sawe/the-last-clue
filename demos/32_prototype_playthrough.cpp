// Demo (CP3, issue #47): the whole prototype loop played by a SCRIPT that sends
// the same mouse/keyboard events a player would (through game::Game, exactly
// like the OpenGL window does):
//   title -> drawer -> torn letter -> painting -> laptop code puzzle -> safe
//   -> evidence board strings -> deduction screen
// Output: output/32_prototype_playthrough.gif / .bmp (key frames)
#include <cmath>
#include <iostream>
#include <string>

#include "demo_common.h"
#include "game/game.h"

namespace {
const double PI = 3.14159265358979;
const double DT = 1.0 / 30;

struct Player {
    game::Game g;
    GifWriter gif{game::Game::W / 2, game::Game::H / 2, 9};   // half size keeps the file small
    Canvas half{game::Game::W / 2, game::Game::H / 2};
    Canvas frame{game::Game::W, game::Game::H};
    PointF mouse{525, 250};
    std::vector<Canvas> keys;
    int tick = 0;

    void drawCursor() {
        const int x = static_cast<int>(mouse.x), y = static_cast<int>(mouse.y);
        for (int i = 0; i < 14; ++i)
            for (int j = 0; j <= i / 2; ++j) frame.setPixel(x + j, y + i, (j == 0 || j == i / 2 || i == 13) ? Palette::Black : Palette::White);
    }
    // Advance time; record every `every`-th frame to keep the GIF small.
    // Half-size frame (every 2nd pixel): no new colours, so the GIF compresses well.
    void record() {
        for (int y = 0; y < half.height(); ++y)
            for (int x = 0; x < half.width(); ++x) half.setPixel(x, y, frame.getPixel(x * 2, y * 2));
        gif.addFrame(half);
    }
    void wait(double seconds, int every = 3) {
        const int n = static_cast<int>(seconds / DT);
        for (int i = 0; i < n; ++i) {
            g.update(DT);
            if (++tick % std::max(every, 3) == 0) { g.render(frame); drawCursor(); record(); }
        }
    }
    void move(PointF to, double seconds = 0.4) {
        const PointF from = mouse;
        const int n = std::max(1, static_cast<int>(seconds / DT));
        for (int i = 1; i <= n; ++i) {
            mouse = {from.x + (to.x - from.x) * i / n, from.y + (to.y - from.y) * i / n};
            g.mouseMove(mouse);
            wait(DT, 3);
        }
    }
    void click(PointF p, double after = 0.3) { move(p); g.mouseDown(p, input::Button::Left); g.mouseUp(p, input::Button::Left); wait(after); }
    void key(input::Key k, double after = 0.05) { g.keyDown(k); wait(after, 1); }
    void type(const std::string& s) { for (char ch : s) { g.character(static_cast<unsigned>(ch)); wait(0.1, 1); } }
    void snap() { g.render(frame); drawCursor(); keys.push_back(frame); }
    PointF view(PointF p) const { return {p.x + game::Game::PanelW, p.y}; }   // view -> window coords
};
}  // namespace

int main() {
    Player p;
    p.g.drawHoverInCanvas = true;                       // the window draws this outline with OpenGL
    p.wait(1.0);
    p.snap();                                           // 1. title
    p.click({525, 300}, 0.5);

    // 2. Drawer -> letter puzzle
    p.move(p.view({90, 290}), 0.5);                     // hover the safe first
    p.wait(0.5);
    p.snap();
    p.click(p.view({454, 281}));
    while (p.g.state().screen != game::Screen::Letter) p.wait(DT * 2);
    p.wait(0.5);
    for (int i = 0; i < 4; ++i) {
        auto& L = p.g.letter();
        const PointF from = p.view(L.pieceScreenCentre(i)), to = p.view(L.solvedScreenCentre(i));
        p.move(from, 0.25);
        p.g.mouseDown(from, input::Button::Left);
        p.move(to, 0.5);
        p.g.mouseUp(to, input::Button::Left);
        const int steps = static_cast<int>(std::lround(std::fabs(L.pieceAngle(i)) / (PI / 12)));
        const double dir = L.pieceAngle(i) > 0 ? -1 : 1;
        for (int s = 0; s < steps; ++s) { p.g.scroll(to, dir); p.wait(0.1, 1); }
        if (i == 2) p.snap();
    }
    while (p.g.state().screen != game::Screen::Explore) p.wait(DT * 2);   // auto-return after solving
    p.wait(0.8);

    // 3. Painting reveals the code
    p.click(p.view({350, 100}));
    while (p.g.state().screen != game::Screen::Painting) p.wait(DT * 2);
    p.wait(2.0);
    p.snap();
    p.click(p.view({400, 250}), 0.6);

    // 4. Laptop: fix the program and run it
    p.click(p.view({575, 150}));
    while (p.g.state().screen != game::Screen::Computer) p.wait(DT * 2);
    p.wait(0.5);
    p.click(p.view({90, 460}), 0.8);                    // RUN the broken code
    p.click(p.view({250, 50 + 6 + 4 * 18 + 8}), 0.2);   // click into line 5
    p.key(input::Key::End);
    p.key(input::Key::Backspace);
    p.key(input::Key::Backspace);
    p.type("144");
    p.click(p.view({90, 460}), 1.2);                    // RUN again -> accepted
    p.snap();
    p.key(input::Key::Escape, 0.6);

    // 5. Safe: turn the dial to 37, 12, 85
    p.click(p.view({92, 290}));
    while (p.g.state().screen != game::Screen::Safe) p.wait(DT * 2);
    p.wait(0.4);
    auto& S = p.g.safe();
    const PointF ctr = p.view(S.dialCentre());
    double a = -PI / 2;
    auto at = [&](double ang) { return PointF{ctr.x + 120 * std::cos(ang), ctr.y + 120 * std::sin(ang)}; };
    p.move(at(a), 0.3);
    for (int n : {37, 12, 85}) {
        const double delta = std::remainder(-2 * PI * n / 100 - (-2 * PI * S.number() / 100), 2 * PI);
        p.g.mouseDown(at(a), input::Button::Left);
        const int steps = std::max(6, static_cast<int>(std::fabs(delta) / 0.15));
        for (int s = 1; s <= steps; ++s) { a += delta / steps; p.mouse = at(a); p.g.mouseMove(p.mouse); p.wait(DT, 1); }
        p.g.mouseUp(at(a), input::Button::Left);
        while (S.number() != n) p.g.keyDown(S.number() < n ? input::Key::Left : input::Key::Right);
        p.key(input::Key::Enter, 0.4);
    }
    p.wait(2.0);
    p.snap();
    p.key(input::Key::Escape, 0.6);

    // 6. Evidence board: tie two strings
    p.click({125, 415}, 0.6);                           // EVIDENCE BOARD button
    auto& B = p.g.board();
    const int pairs[][2] = {{10, 1}, {13, 3}};
    for (const auto& pr : pairs) {
        p.click(p.view(B.pinOf(pr[0])), 0.2);
        p.click(p.view(B.pinOf(pr[1])), 0.4);
    }
    p.wait(0.6);
    p.snap();

    // 7. Deduction (placeholder for the final build)
    p.click({125, 451}, 1.2);
    p.snap();

    const auto& st = p.g.state();
    std::cout << "Final screen: " << game::screenName(st.screen) << "\n"
              << "Evidence: " << st.evidence.size() << "/" << game::EvidenceTotal << "  Strings: " << st.links.size() << "\n"
              << "Flags:";
    for (const auto& f : st.flags) std::cout << " " << f;
    std::cout << "\n";
    if (!p.gif.save("output/32_prototype_playthrough.gif")) return 1;
    std::cout << "Wrote output/32_prototype_playthrough.gif (" << p.gif.frameCount() << " frames)\n";

    Canvas sheet(1050, 1000, Color{30, 32, 40});        // 2 x 4 key frames at half size
    for (size_t k = 0; k < p.keys.size() && k < 8; ++k)
        for (int y = 0; y < 250; ++y)
            for (int x = 0; x < 525; ++x)
                sheet.setPixel(static_cast<int>(k % 2) * 525 + x, static_cast<int>(k / 2) * 250 + y, p.keys[k].getPixel(x * 2, y * 2));
    return demo::save(sheet, "32_prototype_playthrough");
}
