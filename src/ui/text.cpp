// Text and widget helpers — Latiffa (CP3, issue #46)
#include "text.h"

#include <sstream>

#include "animation/animation.h"
#include "core/font.h"
#include "polygon/polygon.h"

namespace ui {

std::vector<std::string> wrap(const std::string& text, int maxChars) {
    std::vector<std::string> out;
    if (maxChars < 1) return out;
    std::istringstream paragraphs(text);
    std::string para;
    while (std::getline(paragraphs, para)) {
        std::istringstream words(para);
        std::string word, line;
        while (words >> word) {
            while (static_cast<int>(word.size()) > maxChars) {          // very long word: hard break
                if (!line.empty()) { out.push_back(line); line.clear(); }
                out.push_back(word.substr(0, static_cast<size_t>(maxChars)));
                word = word.substr(static_cast<size_t>(maxChars));
            }
            if (line.empty()) line = word;
            else if (static_cast<int>(line.size() + 1 + word.size()) <= maxChars) line += " " + word;
            else { out.push_back(line); line = word; }
        }
        out.push_back(line);
    }
    return out;
}

int textBox(Canvas& c, double x, double y, double w, double h, const std::string& text, Color fg, int scale) {
    const int maxChars = static_cast<int>(w) / charW(scale);
    const int maxLines = static_cast<int>(h) / lineH(scale);
    const auto lines = wrap(text, maxChars);
    int n = 0;
    for (const auto& l : lines) {
        if (n >= maxLines) break;
        drawText(c, static_cast<int>(x), static_cast<int>(y) + n * lineH(scale), l, fg, scale);
        ++n;
    }
    return n;
}

void drawButton(Canvas& c, const Button& b, bool enabled, bool hovered, Color base) {
    Color fill = enabled ? base : Color{70, 70, 78};
    if (enabled && hovered) fill = anim::lerp(base, Palette::White, 0.25);   // lighter on hover
    const Polygon r = makeRect(b.x, b.y, b.w, b.h);
    scanlineFill(c, r, fill);
    drawPolygonOutline(c, r, Palette::Black);
    const int scale = b.h >= 30 ? 2 : 1;
    drawText(c, static_cast<int>(b.x + b.w / 2) - textWidth(b.label, scale) / 2,
             static_cast<int>(b.y + b.h / 2) - 7 * scale / 2, b.label, enabled ? Palette::White : Color{150, 150, 160}, scale);
}

void toastBar(Canvas& c, double x, double y, double w, const std::string& message) {
    if (message.empty()) return;
    const double h = 30;
    for (int yy = static_cast<int>(y); yy < static_cast<int>(y + h); ++yy)
        for (int xx = static_cast<int>(x); xx < static_cast<int>(x + w); ++xx)
            c.setPixel(xx, yy, anim::lerp(c.getPixel(xx, yy), Color{10, 10, 14}, 0.82));   // dimmed strip
    drawText(c, static_cast<int>(x + w / 2) - textWidth(message, 2) / 2, static_cast<int>(y) + 8, message,
             Color{255, 230, 150}, textWidth(message, 2) < w - 20 ? 2 : 1);
}

}  // namespace ui
