// Code puzzle validation — Sudheysi (CP3, issue #59)
#include "code_check.h"

#include <algorithm>
#include <cmath>

#include "polygon/polygon.h"
#include "scenes/security_panel.h"

namespace game {

const char* starStarterCode() {
    return "# ALARM KEY - draws my symbol\n"
           "# (it stopped working...)\n"
           "repeat 5 {\n"
           "  forward 120\n"
           "  turn 72\n"
           "}\n";
}

const char* starHint() {
    return "HINT: A 5-POINT STAR TURNS 144 DEGREES AT EACH POINT (5 X 144 = 720 = TWO FULL TURNS).";
}

script::RunOptions puzzleRunOptions() {
    script::RunOptions o;
    o.start = {90, 140};
    o.heading = 0;
    o.stepLimit = 5000;
    return o;
}

const std::vector<script::DrawCmd>& targetStar() {
    static const std::vector<script::DrawCmd> target =
        script::run("repeat 5 {\n forward 120\n turn 144\n}", puzzleRunOptions()).drawing;
    return target;
}

std::vector<bool> rasterise(const std::vector<script::DrawCmd>& drawing, int size, int thickness) {
    Canvas c(size, size, Palette::White);
    script::renderDrawing(c, drawing, Mat3::identity(), ClipRect{0, 0, size - 1.0, size - 1.0}, thickness);
    std::vector<bool> mask(static_cast<size_t>(size) * size);
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x) mask[static_cast<size_t>(y) * size + x] = c.getPixel(x, y) != Palette::White;
    return mask;
}

double intersectionOverUnion(const std::vector<bool>& a, const std::vector<bool>& b) {
    size_t inter = 0, uni = 0;
    for (size_t i = 0; i < a.size() && i < b.size(); ++i) {
        inter += (a[i] && b[i]) ? 1 : 0;
        uni += (a[i] || b[i]) ? 1 : 0;
    }
    return uni == 0 ? 0.0 : static_cast<double>(inter) / uni;
}

CodeCheck checkStar(const script::RunResult& r) {
    CodeCheck out;
    if (!r.ok) {
        out.message = "FIX THE ERROR FIRST";
        return out;
    }
    if (r.drawing.empty()) {
        out.message = "NOTHING WAS DRAWN";
        return out;
    }
    static const std::vector<bool> target = rasterise(targetStar());
    out.iou = intersectionOverUnion(rasterise(r.drawing), target);
    const int pct = static_cast<int>(std::lround(out.iou * 100));
    out.pass = out.iou >= 0.85;
    out.message = out.pass ? "SYMBOL ACCEPTED - ALARM DISABLED (MATCH " + std::to_string(pct) + "%)"
                           : "SYMBOL NOT RECOGNISED (MATCH " + std::to_string(pct) + "%)";
    return out;
}

// =========================== Final build (issue #62) ===========================

CodePuzzle starPuzzle() {
    CodePuzzle p;
    p.file = "ALARM_KEY.CLUE";
    p.goal = "THE ALARM WANTS HER SYMBOL. EDIT THE CODE, THEN RUN.";
    p.starter = starStarterCode();
    p.hint = starHint();
    p.options = puzzleRunOptions();
    p.check = checkStar;
    return p;
}

// ---------------------------------------------------------------- Puzzle A
namespace {
void wiringBackground(Canvas& c, const Mat3& toScreen, bool solved) { drawWiringBoard(c, toScreen, solved); }

double dist(PointF a, PointF b) { return std::hypot(a.x - b.x, a.y - b.y); }

// Orientation of the turn a -> b -> c: > 0 counter-clockwise, < 0 clockwise, 0 collinear
double orient(PointF a, PointF b, PointF c) { return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x); }

bool onSegment(PointF a, PointF b, PointF p) {
    return std::min(a.x, b.x) - 1e-9 <= p.x && p.x <= std::max(a.x, b.x) + 1e-9 &&
           std::min(a.y, b.y) - 1e-9 <= p.y && p.y <= std::max(a.y, b.y) + 1e-9;
}

}  // namespace

// Two segments cross when each one's end points lie on opposite sides of the
// other (orientation test); collinear overlaps and touching ends count too.
bool segmentsCross(PointF a, PointF b, PointF c, PointF d) {
    const double o1 = orient(a, b, c), o2 = orient(a, b, d), o3 = orient(c, d, a), o4 = orient(c, d, b);
    if (((o1 > 0 && o2 < 0) || (o1 < 0 && o2 > 0)) && ((o3 > 0 && o4 < 0) || (o3 < 0 && o4 > 0))) return true;
    if (o1 == 0 && onSegment(a, b, c)) return true;
    if (o2 == 0 && onSegment(a, b, d)) return true;
    if (o3 == 0 && onSegment(c, d, a)) return true;
    if (o4 == 0 && onSegment(c, d, b)) return true;
    return false;
}

CodePuzzle wiringPuzzle() {
    CodePuzzle p;
    p.file = "WIRING.CLUE";
    p.goal = "WIRE EACH TERMINAL TO THE SOCKET OF THE SAME COLOUR. WIRES MUST NOT CROSS.";
    p.starter =
        "# WIRING.CLUE - door lock panel\n"
        "# terminal (left) -> socket of\n"
        "# the SAME colour (right).\n"
        "# wires must not cross!\n"
        "color red\n"
        "line 45 75 255 150\n"
        "color green\n"
        "line 45 150 255 75\n"
        "color blue\n"
        "line 45 225 255 225\n";
    p.hint = "HINT: A WIRE CAN BE SEVERAL LINES. TAKE THE BLUE ONE ROUND THE EDGE: "
             "LINE 45 225 15 225, LINE 15 225 15 25, LINE 15 25 255 25, LINE 255 25 255 75.";
    p.options.start = {150, 150};
    p.options.stepLimit = 5000;
    p.check = checkWiring;
    p.background = wiringBackground;
    return p;
}

CodeCheck checkWiring(const script::RunResult& r) {
    CodeCheck out;
    if (!r.ok) { out.message = "FIX THE ERROR FIRST"; return out; }
    struct Seg { PointF a, b; int wire; };
    const auto& terms = wireTerminals();
    const auto& sockets = wireSockets();

    // 1. Sort the lines into wires by colour (other colours are ignored).
    std::vector<Seg> segs;
    for (const auto& d : r.drawing) {
        if (d.kind != script::DrawCmd::Kind::Line) continue;
        for (size_t w = 0; w < terms.size(); ++w)
            if (d.color == terms[w].colour) segs.push_back({d.pts[0], d.pts[1], static_cast<int>(w)});
    }
    if (segs.empty()) { out.message = "NO WIRES YET - USE COLOR RED / GREEN / BLUE, THEN LINE"; return out; }

    // 2. Follow each wire from its terminal: a segment joins the wire when one of
    //    its ends touches a point the wire already reaches (within 8 px).
    std::vector<bool> attached(segs.size(), false);
    int connected = 0;
    for (size_t w = 0; w < terms.size(); ++w) {
        std::vector<PointF> reach{terms[w].pos};
        for (bool grew = true; grew;) {
            grew = false;
            for (size_t i = 0; i < segs.size(); ++i) {
                if (attached[i] || segs[i].wire != static_cast<int>(w)) continue;
                for (const PointF& q : reach)
                    if (dist(q, segs[i].a) <= 8 || dist(q, segs[i].b) <= 8) {
                        attached[i] = grew = true;
                        reach.push_back(segs[i].a);
                        reach.push_back(segs[i].b);
                        break;
                    }
            }
        }
        const std::string name = terms[w].name;
        if (reach.size() == 1) { out.message = "THE " + name + " WIRE IS NOT CONNECTED TO ITS TERMINAL"; return out; }
        // 3. Which socket does the wire end in? (point-in-polygon on the socket's circle)
        bool home = false;
        for (const auto& s : sockets) {
            const Polygon hole = makeRegularPolygon(s.pos, PortRadius, 24);
            const bool in = std::any_of(reach.begin() + 1, reach.end(), [&](PointF q) { return pointInPolygon(hole, q); });
            if (!in) continue;
            if (s.colour == terms[w].colour) home = true;
            else { out.message = "THE " + name + " WIRE IS PLUGGED INTO THE " + s.name + " SOCKET"; return out; }
        }
        if (!home) { out.message = "THE " + name + " WIRE DOESN'T REACH THE " + name + " SOCKET"; return out; }
        ++connected;
    }
    // 4. No two wires of different colours may cross (short circuit).
    for (size_t i = 0; i < segs.size(); ++i)
        for (size_t j = i + 1; j < segs.size(); ++j) {
            if (!attached[i] || !attached[j] || segs[i].wire == segs[j].wire) continue;
            if (segmentsCross(segs[i].a, segs[i].b, segs[j].a, segs[j].b)) {
                out.message = std::string("SHORT CIRCUIT: THE ") + terms[static_cast<size_t>(segs[i].wire)].name + " AND " +
                              terms[static_cast<size_t>(segs[j].wire)].name + " WIRES CROSS";
                return out;
            }
        }
    out.pass = true;
    out.iou = connected / 3.0;
    out.message = "DOOR LOCK POWERED - ALL 3 WIRES CONNECTED";
    return out;
}

// ---------------------------------------------------------------- Puzzle C
// The victim's last email, Caesar-shifted by 9 (the clock stopped at 9).
const char* emailCipherText() { return "R TWXF HXD CXXT CQN VXWNH. LXWONBB KH OARMJH XA R FRUU. VNNC VN JC 9. - N"; }
const char* emailPlainText() { return "I KNOW YOU TOOK THE MONEY. CONFESS BY FRIDAY OR I WILL. MEET ME AT 9. - E"; }

CodePuzzle emailPuzzle() {
    CodePuzzle p;
    p.file = "MAIL.CLUE";
    p.goal = "HER LAST EMAIL (TO: THE PARTNER) IS SCRAMBLED. FIND THE SHIFT THAT UNSCRAMBLES IT.";
    p.starter =
        "# MAIL.CLUE - to: the partner\n"
        "# every letter was moved along\n"
        "# the alphabet by a secret key\n"
        "let key = 1\n"
        "shift \"R TWXF HXD CXXT\" key\n"
        "shift \"CQN VXWNH. LXWONBB\" key\n"
        "shift \"KH OARMJH XA R FRUU.\" key\n"
        "shift \"VNNC VN JC 9. - N\" key\n";
    p.hint = "HINT: THE CLOCK STOPPED AT 9. SHIFTING BACK BY 9 IS LET KEY = -9 "
             "(OR TRY EVERY KEY: PUT THE SHIFTS IN A REPEAT 26 LOOP AND ADD 1 TO KEY EACH TIME).";
    p.options.stepLimit = 5000;
    p.check = checkEmail;
    p.showsText = true;
    return p;
}

// Pass when the printed lines, joined together, contain the decoded email —
// so both "the right key" and "try every key in a loop" work.
CodeCheck checkEmail(const script::RunResult& r) {
    CodeCheck out;
    if (!r.ok) { out.message = "FIX THE ERROR FIRST"; return out; }
    if (r.printed.empty()) { out.message = "NOTHING WAS PRINTED - USE SHIFT"; return out; }
    std::string all;
    for (const auto& line : r.printed) all += (all.empty() ? "" : " ") + line;
    out.pass = all.find(emailPlainText()) != std::string::npos;
    out.iou = out.pass ? 1 : 0;
    out.message = out.pass ? "EMAIL DECODED - SHE WAS ABOUT TO EXPOSE THE PARTNER" : "STILL SCRAMBLED - TRY ANOTHER KEY";
    return out;
}

}  // namespace game
