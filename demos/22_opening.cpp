// Demo (CP2): openable objects in the crime scene.
//   desk drawer slides out (translate + scale) -> cabinet drawer -> door swings open
//   (scale + shear about the hinge), each with easing, then everything closes again.
// Output: output/22_opening.gif / .bmp
#include "animation/animation.h"
#include "demo_common.h"
#include "scenes/scene_anim.h"

int main() {
    using anim::Ease;
    anim::Track drawer, cabinet, door;
    drawer.key(0.00, 0).key(0.20, 1, Ease::OutBack).key(0.75, 1).key(0.90, 0);
    cabinet.key(0.15, 0).key(0.35, 1, Ease::OutBack).key(0.78, 1).key(0.93, 0);
    door.key(0.30, 0).key(0.60, 1, Ease::InOutCubic).key(0.80, 1).key(1.00, 0);
    return demo::animate("22_opening", 800, 500, 72, [&](Canvas& c, double t) {
        SceneState s;
        s.deskDrawer = drawer.at(t);
        s.cabinetDrawer = cabinet.at(t);
        s.door = door.at(t);
        renderCrimeSceneState(c, s);
    }, 1);
}
