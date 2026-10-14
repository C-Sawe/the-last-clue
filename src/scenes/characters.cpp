// Character line-up — Alvin
// Every character is assembled from the same parts:
//   head / hair / shoes  -> midpoint ellipses (filled)
//   coat, legs, arms     -> polygons (scanline fill); arms are rotated with transforms
//   eyes, buttons, hands -> midpoint circles
//   outlines, tie, mouth -> Bresenham lines
// Each suspect has a signature colour (SuspectColour) used everywhere in the game.
#include <cmath>
#include <string>

#include "../core/font.h"
#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "../transform/transform2d.h"
#include "scenes.h"

namespace {
const Color Skin1{224, 172, 125};
const Color Skin2{141, 85, 54};
const Color Skin3{198, 134, 66};
const Color Outline{30, 26, 24};

Color darker(Color c, int amount = 40) {
    auto d = [amount](uint8_t v) { return static_cast<uint8_t>(v > amount ? v - amount : 0); };
    return {d(c.r), d(c.g), d(c.b)};
}

void poly(Canvas& c, const Polygon& p, Color fill) {
    scanlineFill(c, p, fill);
    drawPolygonOutline(c, p, Outline);
}

struct Look {
    Color coat, trousers, skin, hair;
    bool hat = false, dress = false, longHair = false, glasses = false, tie = false, magnifier = false;
};

// cx = horizontal centre of the character, feet rest on y = 400
void drawCharacter(Canvas& c, int cx, const Look& L) {
    const double x = cx;

    // Legs + shoes
    if (!L.dress) {
        poly(c, Polygon{{{x - 26, 320}, {x - 4, 320}, {x - 6, 392}, {x - 24, 392}}}, L.trousers);
        poly(c, Polygon{{{x + 4, 320}, {x + 26, 320}, {x + 24, 392}, {x + 6, 392}}}, L.trousers);
    } else {
        poly(c, Polygon{{{x - 16, 340}, {x - 6, 340}, {x - 7, 392}, {x - 15, 392}}}, L.skin);
        poly(c, Polygon{{{x + 6, 340}, {x + 16, 340}, {x + 15, 392}, {x + 7, 392}}}, L.skin);
    }
    fillEllipse(c, {cx - 17, 396}, 14, 6, Outline);
    fillEllipse(c, {cx + 17, 396}, 14, 6, Outline);

    // Arms: a rectangle hanging from the shoulder, rotated about the shoulder pivot
    const Polygon armShape = makeRect(-9, 0, 18, 105);
    const PointF lShoulder{x - 40, 212}, rShoulder{x + 40, 212};
    const Polygon lArm = apply(translate(lShoulder.x, lShoulder.y) * rotate(0.22), armShape);
    const Polygon rArm = apply(translate(rShoulder.x, rShoulder.y) * rotate(L.magnifier ? -2.6 : -0.22), armShape);
    poly(c, lArm, darker(L.coat, 20));
    poly(c, rArm, darker(L.coat, 20));
    const PointF lHand = apply(translate(lShoulder.x, lShoulder.y) * rotate(0.22), PointF{0, 110});
    const PointF rHand = apply(translate(rShoulder.x, rShoulder.y) * rotate(L.magnifier ? -2.6 : -0.22), PointF{0, 110});
    fillCircle(c, {static_cast<int>(lHand.x), static_cast<int>(lHand.y)}, 9, L.skin);
    fillCircle(c, {static_cast<int>(rHand.x), static_cast<int>(rHand.y)}, 9, L.skin);

    // Body / coat (a trapezoid; a dress flares out more)
    const double hem = L.dress ? 345 : 330, flare = L.dress ? 62 : 48;
    poly(c, Polygon{{{x - 40, 200}, {x + 40, 200}, {x + flare, hem}, {x - flare, hem}}}, L.coat);
    if (!L.dress) {
        drawLineBresenham(c, {cx, 205}, {cx, static_cast<int>(hem)}, darker(L.coat, 60));   // coat opening
        for (int i = 0; i < 3; ++i) fillCircle(c, {cx + 8, 240 + i * 25}, 3, darker(L.coat, 70));
    }
    if (L.tie) poly(c, Polygon{{{x - 5, 202}, {x + 5, 202}, {x + 7, 255}, {x, 265}, {x - 7, 255}}}, Palette::Black);

    // Neck + head
    poly(c, makeRect(x - 9, 178, 18, 24), L.skin);
    if (L.longHair) fillEllipse(c, {cx, 160}, 36, 50, L.hair);
    fillEllipse(c, {cx, 150}, 28, 35, L.skin);
    drawEllipseMidpoint(c, {cx, 150}, 28, 35, Outline);
    if (!L.hat) fillEllipse(c, {cx, 122}, 26, 12, L.hair);

    // Face
    fillCircle(c, {cx - 10, 146}, 3, Outline);
    fillCircle(c, {cx + 10, 146}, 3, Outline);
    drawLineBresenham(c, {cx, 150}, {cx - 3, 162}, darker(L.skin, 60));
    drawLineBresenham(c, {cx - 8, 170}, {cx + 8, 170}, Color{120, 40, 40});
    if (L.glasses) {
        drawCircleMidpoint(c, {cx - 10, 146}, 7, Outline);
        drawCircleMidpoint(c, {cx + 10, 146}, 7, Outline);
        drawLineBresenham(c, {cx - 3, 146}, {cx + 3, 146}, Outline);
    }

    // Detective hat: wide ellipse brim + trapezoid crown + band
    if (L.hat) {
        fillEllipse(c, {cx, 122}, 48, 9, darker(L.coat, 50));
        poly(c, Polygon{{{x - 26, 121}, {x - 20, 88}, {x + 20, 88}, {x + 26, 121}}}, darker(L.coat, 30));
        poly(c, makeRect(x - 25, 108, 50, 7), Outline);
    }

    // Magnifying glass in the raised right hand
    if (L.magnifier) {
        const Point h{static_cast<int>(rHand.x), static_cast<int>(rHand.y)};
        drawThickLine(c, h, {h.x + 8, h.y - 12}, 5, Color{90, 60, 30});
        fillCircle(c, {h.x + 16, h.y - 26}, 15, Color{200, 225, 240});
        drawCircleMidpoint(c, {h.x + 16, h.y - 26}, 15, Outline);
        drawCircleMidpoint(c, {h.x + 16, h.y - 26}, 16, Outline);
    }
}

void nameplate(Canvas& c, int cx, const std::string& role, const std::string& name, Color col) {
    poly(c, makeRect(cx - 80, 412, 160, 36), Color{245, 240, 225});
    fillCircle(c, {cx - 66, 430}, 7, col);
    drawText(c, cx - 52, 418, role, Color{90, 90, 90});
    drawText(c, cx - 52, 432, name, Outline);
}
}  // namespace

void renderCharacterLineup(Canvas& c) {
    c.clear(Color{214, 216, 222});
    // Police line-up height chart
    for (int i = 0, y = 90; y <= 400; y += 31, ++i) {   // one line per half foot
        drawLineBresenham(c, {0, y}, {c.width() - 1, y}, Color{190, 192, 200});
        if (i % 2 == 0) drawText(c, 6, y - 10, std::to_string(7 - i / 2) + "FT", Color{150, 152, 160});
    }

    Look detective{SuspectColour::Detective, Color{70, 60, 50}, Skin1, Color{60, 40, 20}};
    detective.hat = true;
    detective.magnifier = true;
    Look butler{SuspectColour::Butler, Color{30, 30, 34}, Skin2, Color{30, 25, 20}};
    butler.tie = true;
    Look heiress{SuspectColour::Heiress, Palette::Black, Skin3, Color{90, 50, 20}};
    heiress.dress = true;
    heiress.longHair = true;
    Look partner{SuspectColour::Partner, Color{60, 64, 80}, Skin1, Color{140, 140, 140}};
    partner.glasses = true;
    partner.tie = true;

    drawCharacter(c, 120, detective);
    drawCharacter(c, 340, butler);
    drawCharacter(c, 560, heiress);
    drawCharacter(c, 780, partner);

    nameplate(c, 120, "DETECTIVE", "INSP. KAMAU", SuspectColour::Detective);
    nameplate(c, 340, "SUSPECT 1", "THE BUTLER", SuspectColour::Butler);
    nameplate(c, 560, "SUSPECT 2", "THE HEIRESS", SuspectColour::Heiress);
    nameplate(c, 780, "SUSPECT 3", "THE PARTNER", SuspectColour::Partner);
}
