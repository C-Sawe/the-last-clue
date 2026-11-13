// Canvas -> OpenGL bridge — Caleb (CP3, issue #40)
#include "gl_canvas.h"

#include <GLFW/glfw3.h>   // brings in the system OpenGL header

#include <algorithm>

namespace {
struct Box { double x, y, w, h; };

// Largest rectangle with the canvas aspect ratio that fits inside the window, centred.
Box letterbox(int cw, int ch, int ww, int wh) {
    const double s = std::min(static_cast<double>(ww) / cw, static_cast<double>(wh) / ch);
    const double w = cw * s, h = ch * s;
    return {(ww - w) / 2, (wh - h) / 2, w, h};
}
}  // namespace

GlCanvas::GlCanvas(int width, int height) : w_(width), h_(height), staging_(static_cast<size_t>(width) * height * 3) {
    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // crisp pixels
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);                               // rows are tightly packed RGB
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w_, h_, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

GlCanvas::~GlCanvas() { glDeleteTextures(1, &tex_); }

void GlCanvas::upload(const Canvas& frame) {
    const auto& px = frame.pixels();
    for (size_t i = 0; i < px.size() && i * 3 + 2 < staging_.size(); ++i) {
        staging_[i * 3] = px[i].r;
        staging_[i * 3 + 1] = px[i].g;
        staging_[i * 3 + 2] = px[i].b;
    }
    glBindTexture(GL_TEXTURE_2D, tex_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w_, h_, GL_RGB, GL_UNSIGNED_BYTE, staging_.data());
}

void GlCanvas::draw(int fbW, int fbH) {
    glViewport(0, 0, fbW, fbH);
    glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const Box b = letterbox(w_, h_, fbW, fbH);
    glViewport(static_cast<int>(b.x), static_cast<int>(b.y), static_cast<int>(b.w), static_cast<int>(b.h));

    // Orthographic projection in CANVAS coordinates, y down (like our Canvas).
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w_, h_, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);                                    // one quad = the whole frame
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(static_cast<float>(w_), 0);
    glTexCoord2f(1, 1); glVertex2f(static_cast<float>(w_), static_cast<float>(h_));
    glTexCoord2f(0, 1); glVertex2f(0, static_cast<float>(h_));
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void GlCanvas::outline(const Polygon& p, Color c, float width) {
    glLineWidth(width);
    glColor3ub(c.r, c.g, c.b);
    glBegin(GL_LINE_LOOP);
    for (const auto& v : p.vertices) glVertex2d(v.x, v.y);
    glEnd();
}

void GlCanvas::line(PointF a, PointF b, Color c, float width) {
    glLineWidth(width);
    glColor3ub(c.r, c.g, c.b);
    glBegin(GL_LINES);
    glVertex2d(a.x, a.y);
    glVertex2d(b.x, b.y);
    glEnd();
}

PointF GlCanvas::toCanvas(double wx, double wy, int winW, int winH) const {
    const Box b = letterbox(w_, h_, winW, winH);
    // Window y is measured from the top, like our canvas.
    return {(wx - b.x) / b.w * w_, (wy - b.y) / b.h * h_};
}
