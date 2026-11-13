// Window + render loop — Caleb (CP3, issue #39)
#include "window_app.h"

#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

namespace {
// Everything the GLFW callbacks need, reachable from the window user pointer.
struct AppContext {
    const AppCallbacks* cb = nullptr;
    GlCanvas* gl = nullptr;
};

AppContext& ctx(GLFWwindow* w) { return *static_cast<AppContext*>(glfwGetWindowUserPointer(w)); }

PointF cursorCanvas(GLFWwindow* w) {
    double x, y;
    int ww, wh;
    glfwGetCursorPos(w, &x, &y);
    glfwGetWindowSize(w, &ww, &wh);
    return ctx(w).gl->toCanvas(x, y, ww, wh);
}

input::Key translateKey(int k) {
    switch (k) {
        case GLFW_KEY_ENTER: case GLFW_KEY_KP_ENTER: return input::Key::Enter;
        case GLFW_KEY_ESCAPE:    return input::Key::Escape;
        case GLFW_KEY_BACKSPACE: return input::Key::Backspace;
        case GLFW_KEY_DELETE:    return input::Key::Delete;
        case GLFW_KEY_TAB:       return input::Key::Tab;
        case GLFW_KEY_LEFT:      return input::Key::Left;
        case GLFW_KEY_RIGHT:     return input::Key::Right;
        case GLFW_KEY_UP:        return input::Key::Up;
        case GLFW_KEY_DOWN:      return input::Key::Down;
        case GLFW_KEY_HOME:      return input::Key::Home;
        case GLFW_KEY_END:       return input::Key::End;
        case GLFW_KEY_SPACE:     return input::Key::Space;
        case GLFW_KEY_R:         return input::Key::R;
        case GLFW_KEY_E:         return input::Key::E;
        case GLFW_KEY_B:         return input::Key::B;
        case GLFW_KEY_D:         return input::Key::D;
        case GLFW_KEY_F1:        return input::Key::F1;
        default:                 return input::Key::Unknown;
    }
}

input::Button translateButton(int b) {
    if (b == GLFW_MOUSE_BUTTON_RIGHT) return input::Button::Right;
    if (b == GLFW_MOUSE_BUTTON_MIDDLE) return input::Button::Middle;
    return input::Button::Left;
}

void saveFramebuffer(const std::string& path, int w, int h) {
    std::vector<unsigned char> rgb(static_cast<size_t>(w) * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());
    Canvas shot(w, h);
    for (int y = 0; y < h; ++y)                          // OpenGL rows start at the bottom
        for (int x = 0; x < w; ++x) {
            const size_t i = (static_cast<size_t>(h - 1 - y) * w + x) * 3;
            shot.setPixel(x, y, Color{rgb[i], rgb[i + 1], rgb[i + 2]});
        }
    if (shot.saveBMP(path)) std::cout << "Saved OpenGL screenshot " << path << "\n";
}
}  // namespace

int runWindowApp(const std::string& title, int canvasW, int canvasH, const AppCallbacks& cb) {
    if (!glfwInit()) {
        std::cerr << "Could not start GLFW (no display?)\n";
        return 1;
    }
    GLFWwindow* window = glfwCreateWindow(canvasW, canvasH, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Could not open an OpenGL window\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);                                 // vsync

    {
        GlCanvas gl(canvasW, canvasH);
        Canvas frame(canvasW, canvasH);
        AppContext context{&cb, &gl};
        glfwSetWindowUserPointer(window, &context);

        glfwSetCursorPosCallback(window, [](GLFWwindow* w, double, double) {
            if (ctx(w).cb->mouseMove) ctx(w).cb->mouseMove(cursorCanvas(w));
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int) {
            const PointF p = cursorCanvas(w);
            if (action == GLFW_PRESS && ctx(w).cb->mouseDown) ctx(w).cb->mouseDown(p, translateButton(button));
            if (action == GLFW_RELEASE && ctx(w).cb->mouseUp) ctx(w).cb->mouseUp(p, translateButton(button));
        });
        glfwSetScrollCallback(window, [](GLFWwindow* w, double, double dy) {
            if (ctx(w).cb->scroll && dy != 0) ctx(w).cb->scroll(cursorCanvas(w), dy > 0 ? 1.0 : -1.0);
        });
        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
            if ((action == GLFW_PRESS || action == GLFW_REPEAT) && ctx(w).cb->keyDown)
                ctx(w).cb->keyDown(translateKey(key));
        });
        glfwSetCharCallback(window, [](GLFWwindow* w, unsigned codepoint) {
            if (ctx(w).cb->character) ctx(w).cb->character(codepoint);
        });

        const char* shotPath = std::getenv("LASTCLUE_SCREENSHOT");
        const char* framesEnv = std::getenv("LASTCLUE_FRAMES");
        const int shotFrames = framesEnv ? std::atoi(framesEnv) : 30;

        const double step = 1.0 / 60.0;                  // fixed update rate
        double previous = glfwGetTime(), accumulator = 0;
        int frameCount = 0;
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            const double now = glfwGetTime();
            accumulator += std::min(now - previous, 0.25);   // avoid a spiral after a long pause
            previous = now;
            if (shotPath) accumulator = step;                // deterministic in screenshot mode
            while (accumulator >= step) {
                if (cb.update) cb.update(step);
                accumulator -= step;
            }

            if (cb.render) cb.render(frame);
            gl.upload(frame);
            int fbW, fbH;
            glfwGetFramebufferSize(window, &fbW, &fbH);
            gl.draw(fbW, fbH);
            if (cb.overlay) cb.overlay(gl);

            if (shotPath && ++frameCount >= shotFrames) {
                saveFramebuffer(shotPath, fbW, fbH);
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            glfwSwapBuffers(window);
            if (cb.wantsQuit && cb.wantsQuit()) glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
