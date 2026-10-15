// Demo: torn document puzzle, scattered vs solved.
// Output: output/13_torn_document.bmp
#include "demo_common.h"
#include "scenes/scenes.h"

int main() {
    Canvas a(600, 480), b(600, 480);
    renderTornDocument(a, false);
    renderTornDocument(b, true);

    Canvas c(1210, 524, Color{30, 32, 40});
    demo::title(c, "TORN DOCUMENT PUZZLE", "POLYGON PIECES + INVERSE TRANSFORM MAPPING");
    c.blit(a, 0, 40);
    c.blit(b, 610, 40);
    return demo::save(c, "13_torn_document");
}
