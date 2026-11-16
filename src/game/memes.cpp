// Meme branch: full-screen meme moments played over the game view.
#include "memes.h"

#include <algorithm>
#include <cmath>

#include "animation/animation.h"
#include "core/font.h"
#include "curves/curves.h"
#include "polygon/polygon.h"
#include "primitives/primitives.h"

namespace game {

namespace {
Color darken(Color c, double k) {
    return Color{static_cast<uint8_t>(c.r * k), static_cast<uint8_t>(c.g * k), static_cast<uint8_t>(c.b * k)};
}
Color grey(Color c, double amount) {
    const double g = 0.30 * c.r + 0.59 * c.g + 0.11 * c.b;
    return Color{static_cast<uint8_t>(c.r + (g - c.r) * amount), static_cast<uint8_t>(c.g + (g - c.g) * amount),
                 static_cast<uint8_t>(c.b + (g - c.b) * amount)};
}
// text with a 2-pixel dark outline, so it reads on any picture
void outlinedText(Canvas& c, int x, int y, const std::string& s, Color col, int scale) {
    for (int dy = -2; dy <= 2; dy += 2)
        for (int dx = -2; dx <= 2; dx += 2)
            if (dx || dy) drawText(c, x + dx, y + dy, s, Color{12, 12, 14}, scale);
    drawText(c, x, y, s, col, scale);
}
}  // namespace

void MemeLayer::queue(const MemeClip& c) {
    if (!enabled_) return;
    if (c.name == "@scare") { clips_.push_back(c); return; }        // drawn in code, needs no file
    if (c.video ? !media::video(c.name) : !media::image(c.name)) {   // file missing: still play its sound
        if (!c.sound.empty()) media::playSound(c.sound);
        return;
    }
    clips_.push_back(c);
}

void MemeLayer::clear() {
    clips_.clear();
    media::stopAll();
    started_ = false;
}

double MemeLayer::length() const {
    const MemeClip& c = clips_.front();
    if (c.video && video_) return c.seconds > 0 ? std::min(c.seconds, video_->duration()) : video_->duration();
    return c.seconds > 0 ? c.seconds : 3.0;
}

void MemeLayer::start() {
    const MemeClip& c = clips_.front();
    t_ = 0;
    started_ = true;
    video_ = c.video ? media::video(c.name) : nullptr;
    if (video_) media::playFile(video_->audioPath());
    if (!c.sound.empty()) media::playSound(c.sound);
    if (c.name == "@scare") media::sfx(media::Sfx::Rumble);
    else if (!c.video) media::sfx(media::Sfx::Whoosh);
    screamed_ = false;
}

void MemeLayer::update(double dt) {
    if (clips_.empty()) return;
    if (!started_) start();
    t_ += dt;
    // The scream: the recorded one (assets/memes/scare.mp3) starts 0.38 s early so its loud
    // onset lands with the lunge; without the file, the scream synthesised in code.
    if (clips_.front().name == "@scare" && !screamed_) {
        const bool recorded = media::soundExists("scare");
        if (t_ >= ScareHit - (recorded ? 0.38 : 0.0)) {
            screamed_ = true;
            if (recorded) media::playSound("scare");
            else media::sfx(media::Sfx::Scream);
        }
    }
    if (t_ >= length()) skip();
}

void MemeLayer::skip() {
    if (clips_.empty()) return;
    if (clips_.front().video || clips_.front().name == "@scare") media::stopAll();
    clips_.pop_front();
    started_ = false;
    video_.reset();
    if (!clips_.empty()) start();
}

void MemeLayer::render(Canvas& v) {
    if (clips_.empty()) return;
    if (!started_) start();
    const MemeClip& c = clips_.front();
    const double len = length();

    if (c.name == "@scare") { renderScare(v); return; }
    if (!c.video) {
        // Picture: fade in over 0.5 s, hold, fade out over the last 0.6 s, over the game.
        const double a = std::min({1.0, t_ / 0.5, (len - t_) / 0.6});
        const double alpha = std::max(0.0, a) * 0.85;
        for (int y = 0; y < v.height(); ++y)
            for (int x = 0; x < v.width(); ++x) v.setPixel(x, y, darken(v.getPixel(x, y), 1 - 0.45 * std::max(0.0, a)));
        if (const Canvas* img = media::image(c.name)) {
            const double grow = 0.92 + 0.08 * anim::ease(anim::Ease::OutQuad, std::min(1.0, t_ / 0.8));   // slow zoom in
            const double w = 520 * grow, h = 420 * grow;
            media::drawFit(v, *img, 400 - w / 2, 235 - h / 2, w, h, 0.55, alpha);
        }
        if (!c.caption.empty() && a > 0.5)
            outlinedText(v, 400 - textWidth(c.caption, 3) / 2, 452, c.caption, Color{245, 238, 220}, 3);
        return;
    }

    // Video: darken the game, shake on a jump scare, draw the frame scaled to the view.
    for (int y = 0; y < v.height(); ++y)
        for (int x = 0; x < v.width(); ++x) v.setPixel(x, y, darken(v.getPixel(x, y), 0.18));
    double sx = 0, sy = 0;
    if (c.shake && t_ < 0.8) {
        sx = std::sin(t_ * 70) * 14 * (0.8 - t_);
        sy = std::cos(t_ * 55) * 10 * (0.8 - t_);
    }
    if (video_ && c.crop) {
        renderCropped(v, video_->frameAt(t_));
    } else if (video_) {
        const Canvas& f = video_->frameAt(t_);
        const double grow = c.shake ? 1.0 + 0.25 * std::max(0.0, 0.4 - t_) : 1.0;   // the scare lunges at you
        const double h = 500 * grow, w = 800 * grow;
        media::drawFit(v, f, 400 - w / 2 + sx, 250 - h / 2 + sy, w, h, 0.35, 1.0);
    }
    if (c.shake && t_ < 0.12)                                       // a white flash on the scare
        for (int y = 0; y < v.height(); ++y)
            for (int x = 0; x < v.width(); ++x) v.setPixel(x, y, anim::lerp(v.getPixel(x, y), Color{250, 250, 250}, 0.7 * (1 - t_ / 0.12)));

    if (c.wasted && t_ > 1.2) {
        // GTA-style "wasted": the frame drains to grey, then the word drops in.
        const double g = std::min(1.0, (t_ - 1.2) / 0.8);
        for (int y = 0; y < v.height(); ++y)
            for (int x = 0; x < v.width(); ++x) v.setPixel(x, y, darken(grey(v.getPixel(x, y), g), 1 - 0.25 * g));
        const double drop = anim::ease(anim::Ease::OutBack, std::min(1.0, (t_ - 1.6) / 0.5));
        if (t_ > 1.6) {
            for (int y = 210; y < 300; ++y)                         // dark band behind the word
                for (int x = 0; x < v.width(); ++x) v.setPixel(x, y, anim::lerp(v.getPixel(x, y), Color{10, 10, 12}, 0.55 * drop));
            const int s = 9;
            const int ty = static_cast<int>(anim::lerp(-80, 222, drop));
            outlinedText(v, 400 - textWidth("WASTED", s) / 2, ty, "WASTED", Color{190, 40, 40}, s);
        }
        if (t_ > 2.3) {
            const std::string line = "YOU LET THE MURDERER RUN AWAY";
            outlinedText(v, 400 - textWidth(line, 2) / 2, 312, line, Color{240, 236, 228}, 2);
        }
    }
    drawText(v, 800 - textWidth("CLICK TO SKIP") - 12, 486, "CLICK TO SKIP", Color{150, 150, 160});
}

// ===================== cropped clips (victory and lost scenes) =====================
// Many clips are a small picture inside black bars. Each frame we find the
// picture's bounding box (rows / columns that are not near-black), cut it out,
// and scale it to fill the view's height. The space left at the sides is filled
// with the same picture enlarged, blurred and darkened (instead of the game or
// black), so nothing of the original background shows.
void MemeLayer::renderCropped(Canvas& v, const Canvas& f) {
    auto lit = [&](int x, int y) { const Color c = f.getPixel(x, y); return c.r + c.g + c.b > 75; };
    // The picture is the LONGEST run of lit rows (small gaps allowed), so watermark
    // text sitting in a black bar is not mistaken for part of the picture.
    auto longestRun = [](const std::vector<bool>& on, int& from, int& to) {
        int bestLen = 0, start = -1, gap = 0;
        for (int i = 0; i <= static_cast<int>(on.size()); ++i) {
            const bool o = i < static_cast<int>(on.size()) && on[static_cast<size_t>(i)];
            if (o) { if (start < 0) start = i; gap = 0; }
            else if (start >= 0 && ++gap > 6) {
                const int end = i - gap;
                if (end - start + 1 > bestLen) { bestLen = end - start + 1; from = start; to = end; }
                start = -1; gap = 0;
            }
            if (i == static_cast<int>(on.size()) && start >= 0) {
                const int end = i - 1 - gap;
                if (end - start + 1 > bestLen) { bestLen = end - start + 1; from = start; to = end; }
            }
        }
    };
    int top = 0, bottom = f.height() - 1, left = 0, right = f.width() - 1;
    std::vector<bool> rows(static_cast<size_t>(f.height()));
    for (int y = 0; y < f.height(); ++y) {
        int n = 0;
        for (int x = 0; x < f.width(); x += 4) n += lit(x, y);
        rows[static_cast<size_t>(y)] = n > f.width() / 4 / 5;          // over 20% of the row lit
    }
    longestRun(rows, top, bottom);
    std::vector<bool> cols(static_cast<size_t>(f.width()));
    for (int x = 0; x < f.width(); ++x) {
        int n = 0;
        for (int y = top; y <= bottom; y += 4) n += lit(x, y);
        cols[static_cast<size_t>(x)] = n > (bottom - top) / 4 / 5;
    }
    longestRun(cols, left, right);
    if (bottom - top < 40 || right - left < 40) { top = 0; bottom = f.height() - 1; left = 0; right = f.width() - 1; }

    Canvas pic(right - left + 1, bottom - top + 1);
    for (int y = top; y <= bottom; ++y)
        for (int x = left; x <= right; ++x) pic.setPixel(x - left, y - top, f.getPixel(x, y));

    // Backdrop: the picture shrunk to 40 x 25 and stretched back up = a cheap blur.
    Canvas small(40, 25);
    media::drawCover(small, pic, 0, 0, 40, 25, 0.0);
    for (int y = 0; y < v.height(); ++y)
        for (int x = 0; x < v.width(); ++x) {
            // bilinear sample of the tiny picture
            const double gx = (x + 0.5) / 20.0 - 0.5, gy = (y + 0.5) / 20.0 - 0.5;
            const int x0 = std::clamp(static_cast<int>(std::floor(gx)), 0, 39), y0 = std::clamp(static_cast<int>(std::floor(gy)), 0, 24);
            const int x1 = std::min(x0 + 1, 39), y1 = std::min(y0 + 1, 24);
            const double fx = std::clamp(gx - x0, 0.0, 1.0), fy = std::clamp(gy - y0, 0.0, 1.0);
            const Color a = anim::lerp(small.getPixel(x0, y0), small.getPixel(x1, y0), fx);
            const Color b = anim::lerp(small.getPixel(x0, y1), small.getPixel(x1, y1), fx);
            v.setPixel(x, y, darken(grey(anim::lerp(a, b, fy), 0.3), 0.45));
        }
    // The picture itself, as large as the view allows, with the faded look.
    media::drawFit(v, pic, 0, 0, 800, 500, 0.35, 1.0);
}

// ======================= the jump scare, drawn in code =======================
// A face built from our own primitives (ellipses, polygons, Bezier strands,
// Bresenham cracks), drawn once on its own canvas. The scare is a timeline:
//   0 - 0.65 s  the lights flicker and a rumble starts
//   0.65 s      the face LUNGES at the camera: scaled from 0.35x to 1.6x in
//               0.12 s (inverse-mapped), red flash, screen shake, scream
//   then        it jitters, the picture tears into offset slices (glitch),
//               static noise, and it cuts to black.
namespace {
const Canvas& scareFace() {
    static Canvas face = [] {
        Canvas f(400, 460, Color{0, 0, 0});
        const Color skin{168, 172, 150}, shadow{92, 96, 84}, hole{8, 6, 6};
        fillEllipse(f, {200, 240}, 150, 205, shadow);                     // head
        fillEllipse(f, {200, 228}, 140, 195, skin);
        fillEllipse(f, {200, 330}, 105, 90, Color{140, 144, 124});         // hollow cheeks / jaw
        // stringy black hair: Bezier strands hanging over the forehead
        for (int k = 0; k < 26; ++k) {
            const double x0 = 70 + k * 10.5, sway = ((k * 37) % 23) - 11;
            std::vector<PointF> strand{{x0, 30}, {x0 + sway, 120}, {x0 - sway * 0.6, 150.0 + (k % 5) * 18}};
            drawBezier(f, strand, Color{10, 10, 12}, 30);
            drawBezier(f, {{x0 + 1, 30}, {x0 + sway + 1, 120}, {x0 - sway * 0.6 + 1, 150.0 + (k % 5) * 18}}, Color{10, 10, 12}, 30);
        }
        // sunken eye sockets with tiny glowing pupils
        for (int side : {-1, 1}) {
            const Point e{200 + side * 62, 205};
            fillEllipse(f, e, 50, 38, shadow);
            fillEllipse(f, e, 40, 30, hole);
            fillCircle(f, {e.x + side * 4, e.y + 2}, 7, Color{200, 20, 20});
            fillCircle(f, {e.x + side * 4, e.y + 2}, 3, Color{255, 210, 200});
        }
        // nose: two slits
        drawThickLine(f, {190, 262}, {184, 285}, 3, hole);
        drawThickLine(f, {210, 262}, {216, 285}, 3, hole);
        // the screaming mouth: a tall dark ellipse with jagged teeth
        fillEllipse(f, {200, 360}, 58, 72, hole);
        fillEllipse(f, {200, 368}, 40, 52, Color{60, 8, 10});
        for (int k = 0; k < 7; ++k) {
            const double x = 152 + k * 14;
            scanlineFill(f, Polygon{{{x, 296}, {x + 14, 296}, {x + 7, 318.0 + (k % 2) * 8}}}, Color{220, 214, 190});   // top teeth
            scanlineFill(f, Polygon{{{x, 428}, {x + 14, 428}, {x + 7, 408.0 - (k % 2) * 8}}}, Color{205, 200, 176});   // bottom teeth
        }
        // cracks in the skin
        const Point cracks[][2] = {{{120, 120}, {150, 175}}, {{150, 175}, {138, 210}}, {{285, 140}, {262, 190}}, {{262, 190}, {280, 240}},
                                   {{250, 300}, {290, 330}}, {{110, 300}, {140, 340}}};
        for (const auto& cr : cracks) drawLineBresenham(f, cr[0], cr[1], Color{60, 50, 44});
        return f;
    }();
    return face;
}
unsigned noiseSeed = 99;
int rnd(int n) { noiseSeed = noiseSeed * 1103515245u + 12345u; return static_cast<int>((noiseSeed >> 8) % static_cast<unsigned>(n)); }
}  // namespace

void MemeLayer::renderScare(Canvas& v) {
    const double t = t_;
    if (t < ScareHit) {
        // flickering lights: the room drops to near-black with random flashes
        const bool lit = (static_cast<int>(t * 20) % 5 == 0) || (t > 0.3 && t < 0.36);
        for (int y = 0; y < v.height(); ++y)
            for (int x = 0; x < v.width(); ++x) v.setPixel(x, y, darken(v.getPixel(x, y), lit ? 0.55 : 0.12));
        return;
    }
    const double u = t - ScareHit;
    v.clear(Color{4, 2, 2});
    // the lunge: scale 0.35x -> 1.6x with an overshoot, then a slow creep closer
    const double k = u < 0.14 ? anim::lerp(0.35, 1.6, anim::ease(anim::Ease::OutBack, u / 0.14)) : 1.6 + 0.12 * (u - 0.14);
    const double jx = rnd(21) - 10, jy = rnd(15) - 7;                  // jitter
    const Canvas& face = scareFace();
    const double w = face.width() * k, h = face.height() * k;
    media::drawFit(v, face, 400 - w / 2 + jx, 270 - h / 2 + jy, w, h, 0.0, 1.0);
    // glitch: a few horizontal slices pulled sideways
    for (int g = 0; g < 4; ++g) {
        const int y0 = rnd(470), hh = 6 + rnd(24), off = rnd(61) - 30;
        for (int y = y0; y < std::min(v.height(), y0 + hh); ++y) {
            std::vector<Color> row(static_cast<size_t>(v.width()));
            for (int x = 0; x < v.width(); ++x) row[static_cast<size_t>(x)] = v.getPixel(std::clamp(x - off, 0, v.width() - 1), y);
            for (int x = 0; x < v.width(); ++x) v.setPixel(x, y, row[static_cast<size_t>(x)]);
        }
    }
    // static noise + a red flash on impact
    const double flash = std::max(0.0, 1 - u / 0.25);
    for (int y = 0; y < v.height(); ++y)
        for (int x = 0; x < v.width(); ++x) {
            Color c = v.getPixel(x, y);
            if (rnd(9) == 0) { const uint8_t n = static_cast<uint8_t>(rnd(120)); c = anim::lerp(c, Color{n, n, n}, 0.5); }
            if (flash > 0) c = anim::lerp(c, Color{220, 20, 20}, 0.55 * flash);
            v.setPixel(x, y, c);
        }
    // cut to black at the end
    const double out = std::clamp((t_ - (length() - 0.35)) / 0.35, 0.0, 1.0);
    if (out > 0)
        for (int y = 0; y < v.height(); ++y)
            for (int x = 0; x < v.width(); ++x) v.setPixel(x, y, darken(v.getPixel(x, y), 1 - out));
}

}  // namespace game
