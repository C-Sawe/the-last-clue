#pragma once
// Animated GIF writer — Caleb (CP2, issue #29)
// Pure C++: our own colour palette builder and LZW compressor, no library.
//
// Usage:
//   GifWriter gif(800, 500, 6);          // 6 hundredths of a second per frame
//   for (...) { Canvas c(800, 500); draw(c); gif.addFrame(c); }
//   gif.save("output/anim.gif");
#include <cstdint>
#include <string>
#include <vector>

#include "canvas.h"

class GifWriter {
public:
    GifWriter(int width, int height, int delayCentiseconds = 6, bool loop = true);

    void addFrame(const Canvas& frame);                 // copies the pixels
    void addFrame(const Canvas& frame, int holdFrames); // repeat a frame (pauses)
    size_t frameCount() const { return frames_.size(); }

    // Builds one shared 256-colour palette for all frames, LZW-compresses each
    // frame and writes the file. Returns false if the file can't be written.
    bool save(const std::string& path) const;

private:
    int w_, h_, delay_;
    bool loop_;
    std::vector<std::vector<Color>> frames_;
};
