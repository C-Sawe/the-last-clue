#include "canvas.h"

#include <fstream>

Canvas::Canvas(int width, int height, Color background)
    : w_(width), h_(height), pixels_(static_cast<size_t>(width) * height, background) {}

void Canvas::setPixel(int x, int y, Color c) {
    if (!inBounds(x, y)) return;
    pixels_[static_cast<size_t>(y) * w_ + x] = c;
}

Color Canvas::getPixel(int x, int y) const {
    if (!inBounds(x, y)) return Palette::Black;
    return pixels_[static_cast<size_t>(y) * w_ + x];
}

void Canvas::clear(Color c) {
    for (auto& p : pixels_) p = c;
}

namespace {
void put16(std::ofstream& f, uint16_t v) {
    f.put(static_cast<char>(v & 0xFF));
    f.put(static_cast<char>((v >> 8) & 0xFF));
}
void put32(std::ofstream& f, uint32_t v) {
    for (int i = 0; i < 4; ++i) f.put(static_cast<char>((v >> (8 * i)) & 0xFF));
}
}  // namespace

bool Canvas::saveBMP(const std::string& path) const {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;

    const uint32_t rowSize   = (3u * w_ + 3u) & ~3u;  // rows padded to 4 bytes
    const uint32_t imageSize = rowSize * h_;
    const uint32_t fileSize  = 54u + imageSize;

    // BITMAPFILEHEADER (14 bytes)
    f.put('B'); f.put('M');
    put32(f, fileSize);
    put16(f, 0); put16(f, 0);
    put32(f, 54);
    // BITMAPINFOHEADER (40 bytes)
    put32(f, 40);
    put32(f, static_cast<uint32_t>(w_));
    put32(f, static_cast<uint32_t>(h_));
    put16(f, 1);    // planes
    put16(f, 24);   // bits per pixel
    put32(f, 0);    // no compression
    put32(f, imageSize);
    put32(f, 2835); put32(f, 2835);  // 72 DPI
    put32(f, 0); put32(f, 0);

    // BMP stores rows bottom-up in BGR order.
    const uint32_t padding = rowSize - 3u * w_;
    for (int y = h_ - 1; y >= 0; --y) {
        for (int x = 0; x < w_; ++x) {
            const Color& c = pixels_[static_cast<size_t>(y) * w_ + x];
            f.put(static_cast<char>(c.b));
            f.put(static_cast<char>(c.g));
            f.put(static_cast<char>(c.r));
        }
        for (uint32_t p = 0; p < padding; ++p) f.put(0);
    }
    return static_cast<bool>(f);
}
