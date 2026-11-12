#pragma once
// Window + render loop — Caleb (CP3, issue #39)
//
// Opens a GLFW window with an OpenGL context and runs the game loop:
//     while window open:
//         poll input  -> translate to input::Key / input::Button, canvas coordinates
//         update(dt)  -> fixed time step of 1/60 s (accumulator), so game speed
//                        does not depend on the frame rate
//         render(c)   -> the game draws the frame into a Canvas with OUR algorithms
//         GlCanvas    -> upload the Canvas as a texture, draw it, then the
//                        OpenGL-primitive overlay (hover outlines)
//         swap buffers
#include <functional>
#include <string>

#include "core/canvas.h"
#include "core/keys.h"
#include "gl_canvas.h"

struct AppCallbacks {
    std::function<void(double dt)> update;
    std::function<void(Canvas&)> render;
    std::function<void(GlCanvas&)> overlay;                        // optional
    std::function<void(PointF)> mouseMove;
    std::function<void(PointF, input::Button)> mouseDown;
    std::function<void(PointF, input::Button)> mouseUp;
    std::function<void(PointF, double)> scroll;                    // +1 / -1 per wheel notch
    std::function<void(input::Key)> keyDown;                       // also fires on key repeat
    std::function<void(unsigned)> character;                       // typed text (Unicode code point)
    std::function<bool()> wantsQuit;                               // return true to close
};

// Runs until the window is closed. Returns 0 on success, 1 if no window could be opened.
// Testing aid: if the environment variable LASTCLUE_SCREENSHOT=<file.bmp> is set,
// the app renders LASTCLUE_FRAMES frames (default 30), saves what OpenGL actually
// drew (glReadPixels) to that BMP, and exits.
int runWindowApp(const std::string& title, int canvasW, int canvasH, const AppCallbacks& cb);
