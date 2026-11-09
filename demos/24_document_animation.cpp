// Demo (CP2): torn letter reconstruction — each piece rotates and translates
// back into place (interpolated pose, staggered, eased) and flashes when it snaps.
// Output: output/24_document_animation.gif / .bmp
#include "animation/animation.h"
#include "demo_common.h"
#include "scenes/puzzle_anim.h"

int main() {
    return demo::animate("24_document_animation", 600, 480, 72, [](Canvas& c, double t) {
        renderTornDocumentAt(c, anim::phase(t, 0.08, 0.95));   // short pause at both ends
    }, 20);
}
