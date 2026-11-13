// The Last Clue — CP3 playable prototype (issue #47).
// Our algorithms draw each frame into a Canvas; OpenGL shows it as a texture and
// draws the hover outline with GL_LINE_LOOP; GLFW provides the window and input.
#include "game/game.h"
#include "platform/window_app.h"

int main() {
    game::Game g;
    AppCallbacks cb;
    cb.update = [&](double dt) { g.update(dt); };
    cb.render = [&](Canvas& c) { g.render(c); };
    cb.overlay = [&](GlCanvas& gl) {
        Polygon outline;
        if (g.hoverOutline(outline)) gl.outline(outline, Color{255, 214, 60}, 3.0f);   // OpenGL primitive
    };
    cb.mouseMove = [&](PointF p) { g.mouseMove(p); };
    cb.mouseDown = [&](PointF p, input::Button b) { g.mouseDown(p, b); };
    cb.mouseUp = [&](PointF p, input::Button b) { g.mouseUp(p, b); };
    cb.scroll = [&](PointF p, double d) { g.scroll(p, d); };
    cb.keyDown = [&](input::Key k) { g.keyDown(k); };
    cb.character = [&](unsigned ch) { g.character(ch); };
    return runWindowApp("The Last Clue", game::Game::W, game::Game::H, cb);
}
