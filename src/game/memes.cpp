// Meme branch: full-screen meme moments played over the game view.
#include "memes.h"

#include <algorithm>
#include <cmath>

#include "animation/animation.h"
#include "core/font.h"

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
    if (!c.video) media::sfx(media::Sfx::Whoosh);
}

void MemeLayer::update(double dt) {
    if (clips_.empty()) return;
    if (!started_) start();
    t_ += dt;
    if (t_ >= length()) skip();
}

void MemeLayer::skip() {
    if (clips_.empty()) return;
    if (clips_.front().video) media::stopAll();
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
    if (video_) {
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

}  // namespace game
