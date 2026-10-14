// Demo: polygon representation, scanline fill (convex + concave), flood fill, boundary fill.
// Output: output/04_fill.bmp
#include "demo_common.h"
#include "polygon/polygon.h"
#include "primitives/primitives.h"

int main() {
    Canvas c(1000, 380, Palette::White);
    demo::title(c, "POLYGON FILLING", "SCANLINE / FLOOD / BOUNDARY");

    // 1. Scanline fill — convex hexagon
    Polygon hex = makeRegularPolygon({110, 180}, 80, 6);
    scanlineFill(c, hex, Palette::Wood);
    drawPolygonOutline(c, hex, Palette::Black);
    demo::caption(c, 40, 285, "SCANLINE: CONVEX");

    // 2. Scanline fill — concave star (even-odd pairing of the active edges)
    Polygon star = makeStar({310, 180}, 90, 38, 5);
    scanlineFill(c, star, Palette::Yellow);
    drawPolygonOutline(c, star, Palette::Black);
    demo::caption(c, 240, 285, "SCANLINE: CONCAVE");

    // 3. Scanline fill — irregular "torn paper" polygon (project shape)
    Polygon torn{{{450, 100}, {560, 95}, {552, 125}, {575, 150}, {548, 178}, {570, 210},
                  {555, 262}, {448, 268}, {462, 230}, {440, 195}, {465, 160}, {445, 130}}};
    scanlineFill(c, torn, Palette::Paper);
    drawPolygonOutline(c, torn, Palette::DarkGrey);
    for (int y = 120; y < 250; y += 16) drawLineBresenham(c, {470, y}, {535, y}, Color{120, 120, 150});
    demo::caption(c, 430, 285, "TORN PAPER PIECE");

    // 4. Flood fill — fill the inside of a key outline from one seed pixel
    drawCircleMidpoint(c, {670, 180}, 38, Palette::Black);
    drawCircleMidpoint(c, {670, 180}, 16, Palette::Black);
    floodFill(c, {670, 150}, Palette::Yellow);   // ring between the two circles
    demo::caption(c, 615, 285, "FLOOD FILL (SEED)");

    // 5. Boundary fill — interior has several colours; stops only at the boundary colour
    const Color boundary = Palette::Blue;
    Polygon frame = makeRect(800, 110, 160, 140);
    drawPolygonOutline(c, frame, boundary);
    drawCircleMidpoint(c, {880, 180}, 30, boundary);                          // hole: same boundary colour
    for (int x = 810; x < 960; x += 12) drawLineBresenham(c, {x, 112}, {x, 248}, Palette::Red);  // other colours
    boundaryFill(c, {815, 240}, Color{180, 200, 240}, boundary);
    demo::caption(c, 790, 260, "PAINTS OVER RED, STOPS AT BLUE", demo::Muted);
    demo::caption(c, 800, 285, "BOUNDARY FILL");

    demo::caption(c, 12, 345, "SCANLINE USES AN EDGE TABLE + ACTIVE EDGE TABLE.  FLOOD/BOUNDARY USE AN EXPLICIT STACK (4-CONNECTED).", demo::Muted);
    return demo::save(c, "04_fill");
}
