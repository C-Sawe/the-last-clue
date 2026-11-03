// Demo (CP2): evidence board animation — cards drop in, strings are pulled pin to pin.
// Output: output/19_board_animation.gif / .bmp
#include "demo_common.h"
#include "scenes/board_anim.h"

int main() {
    return demo::animate("19_board_animation", 900, 560, 70,
                         [](Canvas& c, double t) { renderEvidenceBoardAt(c, t); }, 20);
}
