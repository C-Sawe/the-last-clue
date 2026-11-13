// CP3 (issue #39/#40): first OpenGL program — a window that shows our scenes.
// Every frame is still drawn by OUR algorithms into a Canvas; OpenGL displays it
// as a texture and draws the yellow cursor box with its own GL_LINE_LOOP.
//   Keys: 1-4 switch scene (crime scene / walk / evidence board / safe), Esc quits.
#include <cmath>

#include "platform/window_app.h"
#include "scenes/board_anim.h"
#include "scenes/puzzle_anim.h"
#include "scenes/scene_anim.h"
#include "scenes/scenes.h"

int main() {
    const int W = 900, H = 560;
    int scene = 0;
    double t = 0;
    bool quit = false;
    PointF mouse{-100, -100};

    AppCallbacks cb;
    cb.update = [&](double dt) { t += dt; };
    cb.render = [&](Canvas& c) {
        c.clear(Color{30, 32, 40});
        if (scene == 0 || scene == 1) {
            Canvas s(800, 500);
            if (scene == 0) renderCrimeScene(s);
            else renderWalkFrame(s, std::fmod(t / 6.0, 1.0));
            c.blit(s, 50, 30);
        } else if (scene == 2) {
            renderEvidenceBoardAt(c, std::min(1.0, t / 5.0));
        } else {
            Canvas s(420, 420);
            renderSafeOpening(s, t * 0.8, 0);
            c.blit(s, 240, 70);
        }
    };
    cb.overlay = [&](GlCanvas& gl) {                          // OpenGL primitives on top
        gl.outline(makeRect(mouse.x - 12, mouse.y - 12, 24, 24), Color{255, 214, 60});
    };
    cb.mouseMove = [&](PointF p) { mouse = p; };
    cb.character = [&](unsigned ch) {
        if (ch >= '1' && ch <= '4') { scene = static_cast<int>(ch - '1'); t = 0; }
    };
    cb.keyDown = [&](input::Key k) { if (k == input::Key::Escape) quit = true; };
    cb.wantsQuit = [&] { return quit; };
    return runWindowApp("The Last Clue - scene viewer (1-4 to switch)", W, H, cb);
}
