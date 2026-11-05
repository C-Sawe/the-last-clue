// Demo (CP2): detective walk cycle — limbs rotate about hips/shoulders, the
// sprite is scaled into the room and translated along the floor.
// Output: output/21_walk_cycle.gif / .bmp
#include "demo_common.h"
#include "scenes/scene_anim.h"

int main() {
    return demo::animate("21_walk_cycle", 800, 500, 64,
                         [](Canvas& c, double t) { renderWalkFrame(c, t); }, 10);
}
