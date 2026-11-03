// Animated GIF writer — Caleb (CP2, issue #29)
//
// A GIF stores up to 256 colours in a palette; each pixel is an index into it.
// 1. Palette: count every colour used across all frames. Our scenes use flat
//    colours, so usually <= 256 exist and the palette is exact. If there are
//    more, keep the 256 most common and map the rest to the nearest one.
// 2. LZW compression: read indices one by one, growing a dictionary of
//    sequences seen so far, and output the code of the longest known sequence.
//    Codes start 9 bits wide and grow to 12 bits; when the dictionary is full
//    (4096 entries) we output a CLEAR code and start again.
// 3. File layout: header, screen descriptor, global palette, a "loop forever"
//    extension, then for each frame a delay block + image descriptor + data.
#include "gif.h"

#include <algorithm>
#include <fstream>
#include <unordered_map>

namespace {
uint32_t key(Color c) { return (uint32_t(c.r) << 16) | (uint32_t(c.g) << 8) | c.b; }

struct BitWriter {
    std::vector<uint8_t> bytes;
    uint32_t acc = 0;
    int nbits = 0;
    void write(int code, int size) {
        acc |= uint32_t(code) << nbits;
        nbits += size;
        while (nbits >= 8) { bytes.push_back(uint8_t(acc & 0xFF)); acc >>= 8; nbits -= 8; }
    }
    void flush() { if (nbits > 0) { bytes.push_back(uint8_t(acc & 0xFF)); acc = 0; nbits = 0; } }
};

std::vector<uint8_t> lzw(const std::vector<uint8_t>& idx) {
    const int minCode = 8, clear = 256, eoi = 257;
    BitWriter out;
    std::vector<int> dict(4096 * 256, -1);    // dict[prefixCode * 256 + nextIndex] = code
    int codeSize = minCode + 1, next = eoi + 1;

    out.write(clear, codeSize);
    int prefix = idx.empty() ? 0 : idx[0];
    for (size_t i = 1; i < idx.size(); ++i) {
        const int k = idx[i];
        const int found = dict[prefix * 256 + k];
        if (found >= 0) { prefix = found; continue; }       // extend the known sequence
        out.write(prefix, codeSize);                         // emit longest known sequence
        if (next < 4096) {
            dict[prefix * 256 + k] = next++;
            if (next > (1 << codeSize) && codeSize < 12) ++codeSize;
        } else {                                             // dictionary full: start over
            out.write(clear, codeSize);
            std::fill(dict.begin(), dict.end(), -1);
            codeSize = minCode + 1;
            next = eoi + 1;
        }
        prefix = k;
    }
    out.write(prefix, codeSize);
    out.write(eoi, codeSize);
    out.flush();
    return out.bytes;
}

void put16(std::ofstream& f, int v) { f.put(char(v & 0xFF)); f.put(char((v >> 8) & 0xFF)); }
}  // namespace

GifWriter::GifWriter(int width, int height, int delayCentiseconds, bool loop)
    : w_(width), h_(height), delay_(delayCentiseconds), loop_(loop) {}

void GifWriter::addFrame(const Canvas& frame) {
    std::vector<Color> px(static_cast<size_t>(w_) * h_);
    for (int y = 0; y < h_; ++y)
        for (int x = 0; x < w_; ++x) px[static_cast<size_t>(y) * w_ + x] = frame.getPixel(x, y);
    frames_.push_back(std::move(px));
}

void GifWriter::addFrame(const Canvas& frame, int holdFrames) {
    for (int i = 0; i < holdFrames; ++i) addFrame(frame);
}

bool GifWriter::save(const std::string& path) const {
    // ---- 1. palette ----
    std::unordered_map<uint32_t, size_t> counts;
    for (const auto& f : frames_) for (const Color& c : f) ++counts[key(c)];
    std::vector<std::pair<size_t, uint32_t>> byCount;
    for (const auto& kv : counts) byCount.push_back({kv.second, kv.first});
    std::sort(byCount.begin(), byCount.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
    std::vector<Color> palette;
    for (size_t i = 0; i < byCount.size() && i < 256; ++i) {
        const uint32_t k = byCount[i].second;
        palette.push_back({uint8_t(k >> 16), uint8_t(k >> 8), uint8_t(k)});
    }
    while (palette.size() < 256) palette.push_back(Palette::Black);

    std::unordered_map<uint32_t, uint8_t> lookup;
    auto indexOf = [&](const Color& c) -> uint8_t {
        const uint32_t k = key(c);
        auto it = lookup.find(k);
        if (it != lookup.end()) return it->second;
        int best = 0, bestD = 1 << 30;
        for (int i = 0; i < 256; ++i) {
            const int dr = c.r - palette[i].r, dg = c.g - palette[i].g, db = c.b - palette[i].b;
            const int d = dr * dr + dg * dg + db * db;
            if (d < bestD) { bestD = d; best = i; }
        }
        return lookup[k] = uint8_t(best);
    };

    // ---- 2. write the file ----
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f.write("GIF89a", 6);
    put16(f, w_); put16(f, h_);
    f.put(char(0xF7));                      // global colour table, 8 bits/colour, 256 entries
    f.put(0); f.put(0);                     // background index, aspect ratio
    for (const Color& c : palette) { f.put(char(c.r)); f.put(char(c.g)); f.put(char(c.b)); }

    if (loop_) {                            // NETSCAPE2.0 extension: loop forever
        const char ext[] = {'\x21', '\xFF', '\x0B', 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0', '\x03', '\x01', 0, 0, 0};
        f.write(ext, sizeof(ext));
    }

    for (const auto& frame : frames_) {
        f.put(char(0x21)); f.put(char(0xF9)); f.put(4);          // graphic control extension
        f.put(0); put16(f, delay_); f.put(0); f.put(0);
        f.put(char(0x2C));                                        // image descriptor
        put16(f, 0); put16(f, 0); put16(f, w_); put16(f, h_);
        f.put(0);                                                 // no local palette

        std::vector<uint8_t> idx(frame.size());
        for (size_t i = 0; i < frame.size(); ++i) idx[i] = indexOf(frame[i]);
        const std::vector<uint8_t> data = lzw(idx);
        f.put(8);                                                 // LZW minimum code size
        for (size_t i = 0; i < data.size(); i += 255) {           // sub-blocks of <= 255 bytes
            const size_t n = std::min<size_t>(255, data.size() - i);
            f.put(char(n));
            f.write(reinterpret_cast<const char*>(&data[i]), static_cast<std::streamsize>(n));
        }
        f.put(0);
    }
    f.put(char(0x3B));                                            // trailer
    return static_cast<bool>(f);
}
