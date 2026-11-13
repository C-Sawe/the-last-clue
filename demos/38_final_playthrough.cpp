// Demo (final build, issue #54): the WHOLE CASE played by a script that sends
// the same mouse/keyboard events a player would (through game::Game, exactly
// like the OpenGL window does). All six puzzles, a wrong accusation, then the
// right one:
//   letter -> painting -> laptop (star) -> safe -> panel (wiring) -> door ->
//   vial -> laptop (email) -> fingerprints -> board -> deduction -> solved
// Output: output/38_final_playthrough.gif / .bmp (key frames)
//         LASTCLUE_VIDEO=file.mp4 also records a full-size 30 fps video (ffmpeg).
//         LASTCLUE_LOSE=1 instead makes three wrong accusations (tests CASE FAILED).
#include <cmath>
#include <cstdio>
#include <cstdlib>
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
    FILE* video = nullptr;                                    // LASTCLUE_VIDEO: raw frames piped to ffmpeg
    std::string caption;                                      // video only: what is happening, under the game

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
            const bool gifFrame = ++tick % std::max(every, 4) == 0;
            if (gifFrame || video) { g.render(frame); drawCursor(); }
            if (gifFrame) record();
            if (video) writeVideoFrame();
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
    // Video frames are 1050 x 540: the game, plus a caption strip underneath.
    void writeVideoFrame(const Canvas* card = nullptr) {
        static std::vector<unsigned char> rgb;
        Canvas out(1050, 540, Color{14, 14, 18});
        if (card) out.blit(*card, 0, 0);
        else {
            out.blit(frame, 0, 0);
            const int sc = textWidth(caption, 2) <= 1030 ? 2 : 1;
            drawText(out, 525 - textWidth(caption, sc) / 2, 520 - 4 * sc, caption, Color{255, 230, 150}, sc);
        }
        rgb.resize(static_cast<size_t>(out.width()) * out.height() * 3);
        size_t i = 0;
        for (int y = 0; y < out.height(); ++y)
            for (int x = 0; x < out.width(); ++x) {
                const Color c = out.getPixel(x, y);
                rgb[i++] = c.r; rgb[i++] = c.g; rgb[i++] = c.b;
            }
        std::fwrite(rgb.data(), 1, rgb.size(), video);
    }
    // Video only: a full-screen text card held for `seconds`.
    void card(const std::vector<std::string>& lines, double seconds) {
        if (!video) return;
        Canvas c(1050, 540, Color{20, 20, 26});
        int y = 270 - static_cast<int>(lines.size()) * 16;
        for (size_t k = 0; k < lines.size(); ++k) {
            const int sc = k == 0 ? 4 : (textWidth(lines[k], 2) <= 1030 ? 2 : 1);
            drawText(c, 525 - textWidth(lines[k], sc) / 2, y, lines[k], k == 0 ? Palette::White : Color{190, 194, 210}, sc);
            y += k == 0 ? 50 : 26;
        }
        for (int i = 0; i < static_cast<int>(seconds * 30); ++i) writeVideoFrame(&c);
    }
    void say(const std::string& text, double hold = 0) { caption = text; if (video && hold > 0) wait(hold); }
    void snap() { g.render(frame); drawCursor(); keys.push_back(frame); }
    PointF view(PointF p) const { return {p.x + game::Game::PanelW, p.y}; }   // view -> window coords
};
}  // namespace

int main() {
    Player p;
    p.g.drawHoverInCanvas = true;                       // the window draws this outline with OpenGL
    // LASTCLUE_VIDEO=out.mp4 also records every frame (30 fps) through ffmpeg - used for the demo video.
    if (const char* out = std::getenv("LASTCLUE_VIDEO")) {
        const std::string cmd = "ffmpeg -loglevel error -y -f rawvideo -pix_fmt rgb24 -s 1050x540 -r 30 -i - "
                                "-c:v libx264 -pix_fmt yuv420p -crf 23 \"" + std::string(out) + "\"";
        p.video = popen(cmd.c_str(), "w");
    }
    auto waitFor = [&](game::Screen s) { for (int i = 0; i < 300 && p.g.state().screen != s; ++i) p.wait(DT * 2); };
    // Click an object in the study and wait for the detective to walk there.
    auto visit = [&](PointF viewPos, double after) {
        p.click(p.view(viewPos), 0.1);
        for (int i = 0; i < 300 && p.g.explore().busy(); ++i) p.wait(DT * 2);
        p.wait(after);
    };
    auto line = [](int n) { return PointF{250, 56 + 6 + (n - 1) * 18 + 8.0}; };   // editor line n (view coords)

    p.card({"THE LAST CLUE", "A 2D DETECTIVE GAME IN C++", "EVERY PIXEL DRAWN BY OUR OWN ALGORITHMS", "",
            "CALEB KIPCHIRCHIR  -  ALVIN KIMANI  -  SUDHEYSI ALI  -  LATIFFA"}, 5);
    p.say("TITLE: OUR CANVAS IS UPLOADED TO OPENGL AS A TEXTURE EVERY FRAME");
    p.wait(1.5);
    p.snap();                                           // 1. title
    p.click({525, 300}, 0.8);

    p.say("STUDY: POINT-IN-POLYGON PICKING, THE DETECTIVE WALKS (WALK CYCLE)", 1.0);
    // 2. Drawer -> torn letter
    visit({454, 281}, 0);
    waitFor(game::Screen::Letter);
    p.wait(0.5);
    p.say("PUZZLE 1 - TORN LETTER: DRAG (TRANSLATE) + ROTATE PIECES, THEY SNAP INTO PLACE");
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
    }
    waitFor(game::Screen::Explore);
    p.wait(0.8);

    p.say("THE PAINTING SWINGS ON ITS NAIL: ROTATION ABOUT A PIVOT, INVERSE MAPPING");
    // 3. Painting -> the safe code and her symbol
    visit({350, 100}, 0);
    waitFor(game::Screen::Painting);
    p.wait(2.0);
    p.click(p.view({400, 250}), 0.6);

    p.say("PUZZLE 2 - CLUESCRIPT CODE: FIX THE STAR (TURN 144), CHECKED BY IOU");
    // 4. Laptop, puzzle B: fix the star (turn 72 -> 144)
    visit({575, 150}, 0);
    waitFor(game::Screen::Computer);
    p.wait(0.4);
    p.click(p.view({90, 460}), 0.8);                    // RUN the broken code
    p.click(p.view(line(5)), 0.2);
    p.key(input::Key::End);
    p.key(input::Key::Backspace);
    p.key(input::Key::Backspace);
    p.type("144");
    p.click(p.view({90, 460}), 1.2);                    // RUN -> alarm off
    p.snap();
    p.key(input::Key::Escape, 0.6);

    p.say("PUZZLE 3 - SAFE DIAL: ATAN2 MOUSE ANGLE, THE DOOR SWINGS OPEN WITH EASING");
    // 5. Safe: 37 - 12 - 85 -> key + fingerprint
    visit({92, 290}, 0);
    waitFor(game::Screen::Safe);
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
    p.key(input::Key::Escape, 0.6);

    p.say("THE DOOR'S ELECTRONIC LOCK HAS NO POWER...");
    // 6. The door: the electronic lock is dead -> the security panel, puzzle A
    visit({690, 220}, 1.2);                             // "check the panel"
    visit({771, 199}, 0);
    waitFor(game::Screen::Panel);
    p.say("PUZZLE 4 - WIRING CODE: POINT-IN-POLYGON SOCKETS + SEGMENT-INTERSECTION TEST");
    p.wait(0.4);
    p.click(p.view({90, 460}), 1.0);                    // RUN the starter wiring: wrong socket
    p.snap();
    p.click(p.view(line(8)), 0.2);                      // green: 255 75 -> 255 225
    p.key(input::Key::End);
    for (int i = 0; i < 2; ++i) p.key(input::Key::Backspace);
    p.type("225");
    p.click(p.view(line(10)), 0.2);                     // blue: round the edge
    p.key(input::Key::End);
    for (int i = 0; i < 7; ++i) p.key(input::Key::Backspace);
    p.type("15 225");
    for (const char* l : {"line 15 225 15 25", "line 15 25 255 25", "line 255 25 255 75"}) {
        p.key(input::Key::Enter);
        p.type(l);
    }
    p.click(p.view({90, 460}), 1.4);                    // RUN -> door lock powered
    p.snap();
    p.key(input::Key::Escape, 0.6);

    p.say("KEY + POWER: THE DOOR OPENS (SHEAR), THE BACK ROOM HIDES A VIAL");
    // 7. Door (key + power) -> the back room -> the vial
    visit({690, 220}, 1.6);
    visit({700, 254}, 1.2);
    p.snap();

    p.say("PUZZLE 5 - EMAIL CODE: A CAESAR SHIFT IN CLUESCRIPT REVEALS THE MOTIVE");
    // 8. Laptop again, puzzle C: the scrambled email (key 1 -> -9)
    visit({575, 150}, 0);
    waitFor(game::Screen::Computer);
    p.wait(0.3);
    p.key(input::Key::Tab, 0.5);                        // MAIL.CLUE
    p.click(p.view({90, 460}), 0.8);                    // RUN: still scrambled
    p.click(p.view(line(4)), 0.2);
    p.key(input::Key::End);
    p.key(input::Key::Backspace);
    p.type("-9");
    p.click(p.view({90, 460}), 1.4);
    p.snap();
    p.key(input::Key::Escape, 0.6);

    p.say("PUZZLE 6 - FINGERPRINTS: MIDPOINT ELLIPSES, INVERSE-MAPPED ROTATION, IOU MATCH");
    // 9. Fingerprints: pick the partner, turn the print back 60 degrees, compare
    p.click({183, 417}, 0.6);                           // PRINTS button
    auto& P = p.g.prints();
    p.click(p.view(P.cardCentre(2)), 0.4);
    p.move(p.view(P.printCentre()), 0.3);
    for (int i = 0; i < 4; ++i) { p.g.scroll(p.view(P.printCentre()), -1); p.wait(0.35, 1); }
    p.click(p.view({150, 442}), 1.5);                   // COMPARE
    p.snap();

    p.say("EVIDENCE BOARD: DRAG CARDS, STRINGS ARE QUADRATIC BEZIER CURVES");
    // 10. Evidence board: a few strings
    p.click({67, 417}, 0.6);
    auto& B = p.g.board();
    const int pairs[][2] = {{3, 10 + static_cast<int>(game::Evidence::Fingerprint)}, {3, 10 + static_cast<int>(game::Evidence::Email)},
                            {0, 10 + static_cast<int>(game::Evidence::Vial)}};
    for (const auto& pr : pairs) {
        p.click(p.view(B.pinOf(pr[0])), 0.2);
        p.click(p.view(B.pinOf(pr[1])), 0.4);
    }
    p.wait(0.6);
    p.snap();

    p.say("FINAL DEDUCTION: WHO, HOW, WHY - A WRONG ACCUSATION COSTS ONE OF THREE TRIES");
    // 11. Deduction: a wrong accusation first (the heiress, for the will)...
    p.click({125, 451}, 1.2);
    auto& D = p.g.deduction();
    auto accuse = [&](int who, int how, int why) {
        const int picks[3] = {who, how, why};
        for (int q = 0; q < 3; ++q) p.click(p.view(D.cardCentre(q, picks[q])), 0.3);
        p.click(p.view(D.accuseCentre()), 0.2);
    };
    if (std::getenv("LASTCLUE_LOSE")) {                 // test mode: three wrong accusations -> CASE FAILED
        for (int k = 0; k < game::MaxAccusations; ++k) {
            accuse(0, 0, 1);
            p.wait(3.0);
            std::cout << "Accusation " << k + 1 << ": " << (p.g.outcome().verdict().caseClosed ? "CASE FAILED" : "wrong") << "\n";
            p.click(p.view(p.g.outcome().buttonCentre()), 1.0);   // REOPEN THE CASE / PLAY AGAIN
        }
        std::cout << "After PLAY AGAIN: " << game::screenName(p.g.state().screen) << ", evidence " << p.g.state().evidence.size() << "\n";
        if (p.video) pclose(p.video);
        return 0;
    }
    accuse(1, 1, 0);
    p.wait(3.2);
    p.snap();
    p.click(p.view(p.g.outcome().buttonCentre()), 1.0);   // REOPEN THE CASE
    p.say("THE PARTNER, POISON, THE STOLEN MONEY");
    // ...then the right one
    accuse(2, 1, 2);
    p.wait(3.8);
    p.snap();
    p.say("CASE SOLVED: THE STAMP SLAMS DOWN (SCALE + ROTATION, OUTBACK EASING)", 3.0);
    p.wait(1.0);
    p.card({"HOW IT IS BUILT", "LINES: DDA + BRESENHAM   CIRCLES + ELLIPSES: MIDPOINT",
            "FILL: SCANLINE + FLOOD   CLIPPING: COHEN-SUTHERLAND + SUTHERLAND-HODGMAN",
            "CURVES: BEZIER + CATMULL-ROM   TRANSFORMS: 3X3 HOMOGENEOUS MATRICES",
            "ANIMATION: KEYFRAMES, EASING, TRANSITIONS   OPENGL: TEXTURE UPLOAD + GL_LINE_LOOP",
            "CLUESCRIPT: LEXER, RECURSIVE-DESCENT PARSER, TREE-WALKING INTERPRETER",
            "", "GITHUB.COM/C-SAWE/THE-LAST-CLUE"}, 8);
    if (p.video) pclose(p.video);

    const auto& st = p.g.state();
    std::cout << "Final screen: " << game::screenName(st.screen) << "\n"
              << "Evidence: " << st.evidence.size() << "/" << game::EvidenceTotal << "  Strings: " << st.links.size()
              << "  Wrong accusations: " << st.accusations << "\n"
              << "Flags:";
    for (const auto& f : st.flags) std::cout << " " << f;
    std::cout << "\n";
    if (!p.gif.save("output/38_final_playthrough.gif")) return 1;
    std::cout << "Wrote output/38_final_playthrough.gif (" << p.gif.frameCount() << " frames)\n";

    const int rows = static_cast<int>(p.keys.size() + 1) / 2;   // 2 key frames per row, at half size
    Canvas sheet(1050, 250 * rows, Color{30, 32, 40});
    for (size_t k = 0; k < p.keys.size(); ++k)
        for (int y = 0; y < 250; ++y)
            for (int x = 0; x < 525; ++x)
                sheet.setPixel(static_cast<int>(k % 2) * 525 + x, static_cast<int>(k / 2) * 250 + y, p.keys[k].getPixel(x * 2, y * 2));
    std::cout << "Key frames: " << p.keys.size() << "\n";
    return demo::save(sheet, "38_final_playthrough");
}
