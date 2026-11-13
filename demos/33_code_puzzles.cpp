// Demo (final build, issue #62): code puzzles A (security panel wiring) and
// C (encrypted email), each run with its broken starter code and with a fix.
//   Wiring: lines are grouped into wires by colour, each wire is followed from
//           its terminal, point-in-polygon checks which socket it ends in, and
//           a segment-intersection test catches wires that cross.
//   Email:  the `shift` command applies a Caesar shift; the printed text is
//           compared with the expected message.
// Output: output/33_code_puzzles.bmp
#include "demo_common.h"
#include "game/code_check.h"
#include "polygon/polygon.h"
#include "scenes/security_panel.h"
#include "script/script.h"
#include "ui/text.h"

namespace {
const char* WiringFix =
    "color red\n"
    "line 45 75 255 150\n"
    "color green\n"
    "line 45 150 255 225\n"
    "color blue\n"
    "line 45 225 15 225\n"
    "line 15 225 15 25\n"
    "line 15 25 255 25\n"
    "line 255 25 255 75\n";

const char* EmailFix =
    "let key = -9\n"
    "shift \"R TWXF HXD CXXT\" key\n"
    "shift \"CQN VXWNH. LXWONBB\" key\n"
    "shift \"KH OARMJH XA R FRUU.\" key\n"
    "shift \"VNNC VN JC 9. - N\" key\n";

void wiringPanel(Canvas& c, int ox, int oy, const char* code, const char* label) {
    const game::CodePuzzle p = game::wiringPuzzle();
    const script::RunResult r = script::run(code, p.options);
    const game::CodeCheck check = p.check(r);
    const Mat3 M = translate(ox, oy);
    drawWiringBoard(c, M, check.pass);
    script::renderDrawing(c, r.drawing, M, ClipRect{static_cast<double>(ox), static_cast<double>(oy), ox + 299.0, oy + 299.0}, 4);
    demo::caption(c, ox, oy + 308, label);
    ui::textBox(c, ox, oy + 324, 300, 30, check.message, check.pass ? Color{30, 140, 60} : Palette::Red);
}

void emailPanel(Canvas& c, int ox, int oy, const char* code, const char* label) {
    const game::CodePuzzle p = game::emailPuzzle();
    const script::RunResult r = script::run(code, p.options);
    const game::CodeCheck check = p.check(r);
    scanlineFill(c, makeRect(ox, oy, 300, 300), Color{20, 24, 30});
    int y = oy + 12;
    for (const auto& line : r.printed) {
        for (const auto& part : ui::wrap(line, 23)) {
            drawText(c, ox + 10, y, part, check.pass ? Color{120, 230, 140} : Color{150, 230, 160}, 2);
            y += 20;
        }
        y += 6;
    }
    demo::caption(c, ox, oy + 308, label);
    ui::textBox(c, ox, oy + 324, 300, 30, check.message, check.pass ? Color{30, 140, 60} : Palette::Red);
}
}  // namespace

int main() {
    Canvas c(1290, 430, Palette::White);
    demo::title(c, "CODE PUZZLES A + C", "WIRES: POINT-IN-POLYGON + SEGMENT INTERSECTION   EMAIL: CAESAR SHIFT");
    wiringPanel(c, 10, 50, game::wiringPuzzle().starter.c_str(), "A: STARTER CODE");
    wiringPanel(c, 330, 50, WiringFix, "A: FIXED - BLUE GOES ROUND THE EDGE");
    emailPanel(c, 650, 50, game::emailPuzzle().starter.c_str(), "C: STARTER CODE (KEY = 1)");
    emailPanel(c, 970, 50, EmailFix, "C: FIXED (KEY = -9)");

    // A few sanity checks of the crossing test, printed for the report
    const bool crossX = game::segmentsCross({0, 0}, {10, 10}, {0, 10}, {10, 0});
    const bool parallel = game::segmentsCross({0, 0}, {10, 0}, {0, 5}, {10, 5});
    std::cout << "segments cross (X shape): " << crossX << "   parallel: " << parallel << "\n";
    return demo::save(c, "33_code_puzzles");
}
