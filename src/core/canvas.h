#pragma once
// Canvas: the only "drawing surface" in the project.
// Every algorithm draws by calling setPixel() — no graphics library calls.
#include <cstdint>
#include <string>
#include <vector>

struct Color {
    uint8_t r = 0, g = 0, b = 0;
    bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b; }
    bool operator!=(const Color& o) const { return !(*this == o); }
};

struct Point {
    int x = 0, y = 0;
};

struct PointF {
    double x = 0.0, y = 0.0;
};

// A handy palette so scenes use consistent colours.
namespace Palette {
    const Color Black     {  0,   0,   0};
    const Color White     {255, 255, 255};
    const Color Red       {200,  40,  40};
    const Color Green     { 40, 160,  70};
    const Color Blue      { 40,  80, 200};
    const Color Yellow    {230, 200,  50};
    const Color Grey      {128, 128, 128};
    const Color DarkGrey  { 60,  60,  60};
    const Color Wood      {120,  80,  45};
    const Color Paper     {240, 230, 200};
    const Color Cork      {190, 145,  95};
    const Color StringRed {180,  20,  30};
}

class Canvas {
public:
    Canvas(int width, int height, Color background = Palette::White);

    int width()  const { return w_; }
    int height() const { return h_; }

    // Origin (0,0) is the TOP-LEFT corner; y grows downwards.
    // Out-of-bounds writes are silently ignored.
    void  setPixel(int x, int y, Color c);
    Color getPixel(int x, int y) const;
    bool  inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < w_ && y < h_; }

    void clear(Color c);

    // Writes a 24-bit uncompressed BMP (opens natively on Windows/macOS/Linux).
    bool saveBMP(const std::string& path) const;

private:
    int w_, h_;
    std::vector<Color> pixels_;
};
