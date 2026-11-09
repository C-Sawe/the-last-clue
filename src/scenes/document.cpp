// Torn document puzzle — Sudheysi
//
// 1. The intact letter is drawn once onto its own small canvas (the "texture").
// 2. Two jagged tear lines cut it into 4 irregular polygons.
// 3. Each piece gets a transform M (rotate + translate). To draw it we:
//      - transform the piece's polygon with M,
//      - visit every pixel of the transformed piece (point-in-polygon test),
//      - map that pixel BACK with inverse(M) to find which letter pixel it shows.
//    Mapping backwards (destination -> source) guarantees no holes appear.
// The scattered state is the puzzle; the solved state is where the player
// must move and rotate the pieces back to.
#include <cmath>
#include <vector>

#include "../core/font.h"
#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "../animation/animation.h"
#include "../transform/transform2d.h"
#include "puzzle_anim.h"
#include "scenes.h"

namespace {
const int W = 260, H = 330;  // letter size

Canvas makeLetter() {
    Canvas l(W, H, Palette::Paper);
    for (int y = 70; y < H - 20; y += 22) drawLineBresenham(l, {14, y}, {W - 14, y}, Color{205, 200, 175});
    drawLineBresenham(l, {34, 0}, {34, H - 1}, Color{220, 150, 150});                 // margin
    drawText(l, 48, 22, "CONFIDENTIAL", Color{170, 30, 30}, 2);
    const char* lines[] = {"MEET ME AT THE", "OLD MILL AT 9PM.", "BRING THE KEY", "TO THE SAFE.", "TELL NO ONE.", "", "THE CODE IS IN", "THE PAINTING."};
    int y = 56;
    for (const char* s : lines) { drawText(l, 44, y, s, Color{25, 35, 90}, 2); y += 22; }
    drawText(l, 150, 290, "- R.", Color{25, 35, 90}, 2);
    fillEllipse(l, {200, 250}, 26, 18, Color{205, 180, 140});                         // coffee stain
    fillEllipse(l, {200, 250}, 20, 13, Palette::Paper);
    return l;
}

// Jagged tears that meet at the centre point (W/2, H/2)
std::vector<PointF> verticalTop() { return {{128, 0}, {136, 30}, {122, 62}, {138, 95}, {125, 130}, {W / 2.0, H / 2.0}}; }
std::vector<PointF> verticalBottom() { return {{W / 2.0, H / 2.0}, {138, 200}, {121, 236}, {137, 270}, {124, 302}, {133, H}}; }
std::vector<PointF> horizontalLeft() { return {{0, 168}, {30, 158}, {62, 175}, {95, 157}, {W / 2.0, H / 2.0}}; }
std::vector<PointF> horizontalRight() { return {{W / 2.0, H / 2.0}, {165, 175}, {196, 156}, {228, 172}, {W, 160}}; }

std::vector<Polygon> pieces() {
    auto rev = [](std::vector<PointF> v) { return std::vector<PointF>(v.rbegin(), v.rend()); };
    auto join = [](std::initializer_list<std::vector<PointF>> parts) {
        Polygon p;
        for (const auto& part : parts) p.vertices.insert(p.vertices.end(), part.begin(), part.end());
        return p;
    };
    const auto vt = verticalTop(), vb = verticalBottom(), hl = horizontalLeft(), hr = horizontalRight();
    return {
        join({{{0, 0}}, vt, rev(hl)}),                      // top-left
        join({{{W, 0}}, rev(hr), rev(vt)}),                 // top-right  (W,0) -> right tear -> centre -> top tear
        join({hl, vb, {{0, H}}}),                           // bottom-left
        join({hr, {{W, H}}, rev(vb)}),                      // bottom-right
    };
}

// Draw one piece of `letter` transformed by M.
void drawPiece(Canvas& c, const Canvas& letter, const Polygon& piece, const Mat3& M) {
    const Polygon dst = apply(M, piece);
    const Mat3 back = inverse(M);

    // soft shadow: same shape filled dark, offset by 5 px
    scanlineFill(c, apply(translate(5, 6), dst), Color{60, 40, 25});

    double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (const auto& v : dst.vertices) {
        minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y); maxY = std::max(maxY, v.y);
    }
    for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y)
        for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); ++x) {
            const PointF p{x + 0.5, y + 0.5};
            if (!pointInPolygon(dst, p)) continue;
            const PointF s = apply(back, p);
            c.setPixel(x, y, letter.getPixel(static_cast<int>(s.x), static_cast<int>(s.y)));
        }
    drawPolygonOutline(c, dst, Color{150, 135, 100});
}
}  // namespace

void renderTornDocument(Canvas& c, bool solved) { renderTornDocumentAt(c, solved ? 1.0 : 0.0); }

// CP2 (issue #36): pieces fly back into place.
// Each piece has a scattered pose (angle a, offset d) and a solved pose (angle 0,
// a 2 px gap g). For local progress u (eased, staggered per piece):
//     M(u) = translate(lerp(d, g, u)) * rotateAbout(lerp(a, 0, u), centre) * translate(origin)
// u = 0 is exactly the CP1 puzzle, u = 1 exactly the CP1 solution.
void renderTornDocumentAt(Canvas& c, double progress) {
    c.clear(Color{112, 74, 44});                                         // desk
    for (int y = 0; y < c.height(); y += 36) drawLineBresenham(c, {0, y}, {c.width() - 1, y}, Color{96, 62, 36});

    const Canvas letter = makeLetter();
    const auto parts = pieces();
    const double ox = (c.width() - W) / 2.0, oy = (c.height() - H) / 2.0 + 10;

    // Where each piece lies on the desk: {rotation, dx, dy}
    struct Pose { double angle, dx, dy; };
    const Pose scattered[] = {{-0.35, -125, -45}, {0.5, 108, -38}, {0.25, -135, 30}, {-0.6, 108, 22}};
    for (size_t i = 0; i < parts.size(); ++i) {
        const double start = 0.18 * i, end = start + 0.46;               // staggered
        const double raw = anim::phase(progress, start, end);
        const double u = (raw >= 1) ? 1.0 : anim::ease(anim::Ease::InOutCubic, raw);

        PointF centre{0, 0};
        for (const auto& v : parts[i].vertices) { centre.x += v.x; centre.y += v.y; }
        centre = {centre.x / parts[i].vertices.size() + ox, centre.y / parts[i].vertices.size() + oy};

        // tiny gaps along the tears so the solved pieces read as separate paper
        const double gx = (i % 2 == 0) ? -2 : 2, gy = (i < 2) ? -2 : 2;
        Mat3 M;
        if (u <= 0)      M = translate(scattered[i].dx, scattered[i].dy) * rotateAbout(scattered[i].angle, centre) * translate(ox, oy);
        else if (u >= 1) M = translate(gx, gy) * translate(ox, oy);
        else M = translate(anim::lerp(scattered[i].dx, gx, u), anim::lerp(scattered[i].dy, gy, u)) *
                 rotateAbout(anim::lerp(scattered[i].angle, 0, u), centre) * translate(ox, oy);
        drawPiece(c, letter, parts[i], M);

        // "Snap" feedback: a green outline flashes for a moment when a piece lands
        const double snap = anim::phase(progress, end, end + 0.08);
        if (snap > 0 && snap < 1) drawPolygonOutline(c, apply(M, parts[i]), Color{90, 220, 110});
    }

    const bool solved = progress >= 1;
    const std::string status = solved ? "SOLVED - LETTER RESTORED" : (progress <= 0 ? "PUZZLE - 4 TORN PIECES" : "RECONSTRUCTING...");
    drawText(c, 16, c.height() - 26, status, solved ? Color{140, 230, 140} : Palette::Yellow, 2);
}
