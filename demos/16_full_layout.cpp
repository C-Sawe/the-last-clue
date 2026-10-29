// Demo: the planned game window — Case File panel (left) + Crime Scene viewport (right).
// Output: output/16_full_layout.bmp
#include "demo_common.h"
#include "scenes/scenes.h"

int main() {
    Canvas panel(250, 500), scene(800, 500);
    renderCaseFilePanel(panel);
    renderCrimeScene(scene);

    Canvas c(1050, 500);
    c.blit(panel, 0, 0);
    c.blit(scene, 250, 0);
    return demo::save(c, "16_full_layout");
}
