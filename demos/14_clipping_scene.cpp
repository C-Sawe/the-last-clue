// Demo: clipping applied to the game — viewport + magnifier.
// Output: output/14_clipping_scene.bmp
#include "demo_common.h"
#include "scenes/scenes.h"

int main() {
    Canvas c(940, 450);
    renderClippingDemo(c);
    demo::title(c, "CLIPPING IN THE GAME", "SUTHERLAND-HODGMAN + COHEN-SUTHERLAND");
    return demo::save(c, "14_clipping_scene");
}
