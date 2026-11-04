// Demo (CP2): the safe puzzle solved — the dial turns through the combination
// 37 - 12 - 85 (rotation about the dial centre, eased, with pauses), then the
// door swings open about its hinge (scale + shear) to reveal the key.
// Output: output/23_safe_animation.gif / .bmp
#include "animation/animation.h"
#include "demo_common.h"
#include "scenes/puzzle_anim.h"

int main() {
    using anim::Ease;
    const double PI = 3.14159265358979;
    // Dial position measured in "numbers" (100 per turn). Going past 100 = a full extra turn.
    anim::Track number, door;
    number.key(0.00, 0)
          .key(0.24, 137, Ease::InOutCubic).key(0.30, 137)      // right, past 0, stop at 37
          .key(0.44, 112, Ease::InOutCubic).key(0.50, 112)      // back left to 12
          .key(0.66, 185, Ease::InOutCubic).key(0.72, 185);     // right again to 85
    door.key(0.74, 0).key(0.98, 1, Ease::OutBack);
    return demo::animate("23_safe_animation", 420, 420, 84, [&](Canvas& c, double t) {
        renderSafeOpening(c, -2 * PI * number.at(t) / 100.0, door.at(t));
    }, 20);
}
