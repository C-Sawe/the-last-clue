// Demo: the crime scene, built from our primitives.  Output: output/10_crime_scene.bmp
#include "demo_common.h"
#include "scenes/scenes.h"

int main() {
    Canvas c(800, 500, Palette::Black);
    renderCrimeScene(c);
    return demo::save(c, "10_crime_scene");
}
