// Demo: detective + three colour-coded suspects.  Output: output/11_characters.bmp
#include "demo_common.h"
#include "scenes/scenes.h"

int main() {
    Canvas c(900, 460, Palette::White);
    renderCharacterLineup(c);
    return demo::save(c, "11_characters");
}
