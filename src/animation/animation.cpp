// Animation core — Latiffa (CP2, issue #30)
#include "animation.h"

#include <algorithm>
#include <cmath>

namespace anim {

double clamp01(double t) { return t < 0 ? 0 : (t > 1 ? 1 : t); }

double lerp(double a, double b, double t) { return a + (b - a) * t; }

PointF lerp(PointF a, PointF b, double t) { return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)}; }

Color lerp(Color a, Color b, double t) {
    auto ch = [t](uint8_t x, uint8_t y) {
        return static_cast<uint8_t>(std::lround(x + (y - x) * clamp01(t)));
    };
    return {ch(a.r, b.r), ch(a.g, b.g), ch(a.b, b.b)};
}

double phase(double t, double a, double b) {
    if (b <= a) return t >= b ? 1.0 : 0.0;
    return clamp01((t - a) / (b - a));
}

double ease(Ease e, double t) {
    t = clamp01(t);
    switch (e) {
        case Ease::Linear:  return t;
        case Ease::InQuad:  return t * t;
        case Ease::OutQuad: return 1 - (1 - t) * (1 - t);
        case Ease::InOutCubic:
            return t < 0.5 ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3) / 2;
        case Ease::OutBack: {
            const double c1 = 1.70158, c3 = c1 + 1;          // overshoot amount
            return 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
        }
        case Ease::OutBounce: {
            const double n = 7.5625, d = 2.75;               // piecewise parabolas
            if (t < 1 / d) return n * t * t;
            if (t < 2 / d) { t -= 1.5 / d;   return n * t * t + 0.75; }
            if (t < 2.5 / d) { t -= 2.25 / d; return n * t * t + 0.9375; }
            t -= 2.625 / d;
            return n * t * t + 0.984375;
        }
    }
    return t;
}

Track& Track::key(double time, double value, Ease e) {
    keys_.push_back({time, value, e});
    std::sort(keys_.begin(), keys_.end(), [](const Key& a, const Key& b) { return a.time < b.time; });
    return *this;
}

double Track::at(double time) const {
    if (keys_.empty()) return 0;
    if (time <= keys_.front().time) return keys_.front().value;
    if (time >= keys_.back().time) return keys_.back().value;
    for (size_t i = 0; i + 1 < keys_.size(); ++i) {
        const Key& a = keys_[i];
        const Key& b = keys_[i + 1];
        if (time <= b.time) {
            const double local = phase(time, a.time, b.time);
            return lerp(a.value, b.value, ease(a.easeToNext, local));
        }
    }
    return keys_.back().value;
}

double Track::duration() const { return keys_.empty() ? 0 : keys_.back().time; }

}  // namespace anim
