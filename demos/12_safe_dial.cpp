// Demo: the safe dial at two rotations (same code, different angle).
// Output: output/12_safe_dial.bmp
#include "demo_common.h"
#include "scenes/scenes.h"

int main() {
    const double PI = 3.14159265358979;
    Canvas a(420, 420), b(420, 420);
    renderSafeDial(a, 0.0);
    renderSafeDial(b, -2 * PI * 37 / 100);   // turned so 37 sits under the pointer

    Canvas c(880, 470, Color{30, 32, 40});
    demo::title(c, "SAFE DIAL", "ROTATION ABOUT THE DIAL CENTRE");
    c.blit(a, 10, 42);
    c.blit(b, 450, 42);
    return demo::save(c, "12_safe_dial");
}
