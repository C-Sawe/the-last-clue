// Demo (CP2): scene transitions between the crime scene and the evidence board.
//   wipe (clip window grows) -> iris (circle) -> fade through black -> cross-fade
// Output: output/20_transitions.gif / .bmp
#include "animation/transitions.h"
#include "demo_common.h"
#include "scenes/scenes.h"

int main() {
    const int W = 800, H = 500;
    Canvas scene(W, H), board900(900, 560), board(W, H);
    renderCrimeScene(scene);
    renderEvidenceBoard(board900);
    for (int y = 0; y < H; ++y)                       // fit the 900x560 board into 800x500
        for (int x = 0; x < W; ++x) board.setPixel(x, y, board900.getPixel(x * 900 / W, y * 560 / H));

    const char* names[] = {"WIPE (CLIPPING)", "IRIS (CIRCLE)", "FADE THROUGH BLACK", "CROSS-FADE (LERP)"};
    return demo::animate("20_transitions", W, H, 96, [&](Canvas& out, double t) {
        const int part = std::min(3, static_cast<int>(t * 4));
        const double local = t * 4 - part;                      // 0 -> 1 within each part
        const bool forward = (part % 2 == 0);                   // alternate direction
        const Canvas& from = forward ? scene : board;
        const Canvas& to = forward ? board : scene;
        switch (part) {
            case 0: anim::wipe(out, from, to, local); break;
            case 1: anim::iris(out, from, to, local, {400, 250}); break;
            case 2: anim::fadeThroughBlack(out, from, to, local); break;
            default: anim::crossFade(out, from, to, local); break;
        }
        for (int y = 0; y < 22; ++y)
            for (int x = 0; x < 230; ++x) out.setPixel(x, y, Color{20, 20, 26});
        drawText(out, 8, 8, names[part], Palette::White);
    }, 1);
}
