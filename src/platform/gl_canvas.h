#pragma once
// Canvas -> OpenGL bridge — Caleb (CP3, issue #40)
//
// Our own algorithms still draw every frame into a Canvas (CPU pixels).
// GlCanvas puts that frame on screen with OpenGL:
//   1. upload(): copy the Canvas pixels into an OpenGL texture (glTexSubImage2D)
//   2. draw():   draw one textured quad that fills the window, keeping the
//                aspect ratio (letterboxing) and using nearest filtering so our
//                pixel art stays crisp
// On top of the frame, OpenGL draws vector overlays with its own primitives
// (GL_LINE_LOOP / GL_LINES) in canvas coordinates — e.g. hover outlines.
#include "core/canvas.h"
#include "polygon/polygon.h"

class GlCanvas {
public:
    GlCanvas(int width, int height);
    ~GlCanvas();
    GlCanvas(const GlCanvas&) = delete;
    GlCanvas& operator=(const GlCanvas&) = delete;

    void upload(const Canvas& frame);
    // Draws the texture letterboxed into a framebuffer of fbW x fbH pixels and
    // sets up an orthographic projection in canvas coordinates for overlays.
    void draw(int fbW, int fbH);

    // OpenGL-primitive overlays, in canvas coordinates (call after draw()).
    void outline(const Polygon& p, Color c, float width = 2.0f);
    void line(PointF a, PointF b, Color c, float width = 1.0f);

    // Window -> canvas coordinates (accounts for the letterbox).
    PointF toCanvas(double windowX, double windowY, int winW, int winH) const;

    int width() const { return w_; }
    int height() const { return h_; }

private:
    int w_, h_;
    unsigned tex_ = 0;
    std::vector<unsigned char> staging_;
};
