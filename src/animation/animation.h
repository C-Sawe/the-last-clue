#pragma once
// Animation core — Latiffa (CP2, issue #30)
//
// Every animation in the game is "a value that changes with time":
//   a position (translate), an angle (rotate), a size (scale), a colour.
// This module gives us three tools for that:
//   1. Interpolation: lerp — blend between two values by t in [0,1].
//   2. Easing: reshape t so motion starts/stops smoothly instead of robotically.
//   3. Keyframe tracks: "at 0.0s be here, at 0.5s be there…", sampled at any time.
// Scenes then feed the animated values into our transform matrices (Mat3).
#include <vector>

#include "../core/canvas.h"

namespace anim {

// ---- Interpolation ---------------------------------------------------------
double clamp01(double t);
double lerp(double a, double b, double t);              // a + (b - a) t
PointF lerp(PointF a, PointF b, double t);
Color  lerp(Color a, Color b, double t);                // per-channel blend (fades)

// Local progress of a sub-animation that runs between global times a and b:
// 0 before a, 1 after b, linear in between. Lets one t drive a whole sequence.
double phase(double t, double a, double b);

// ---- Easing ----------------------------------------------------------------
enum class Ease {
    Linear,     // constant speed
    InQuad,     // starts slow, speeds up          t^2
    OutQuad,    // starts fast, slows down         1-(1-t)^2
    InOutCubic, // slow - fast - slow              smooth start and stop
    OutBack,    // overshoots a little then settles (a card "landing")
    OutBounce,  // bounces at the end (a dropped object)
};
double ease(Ease e, double t);

// ---- Keyframes -------------------------------------------------------------
// A Track holds (time, value) keys. at(time) finds the two keys around `time`
// and interpolates between them with the easing of the earlier key.
struct Key {
    double time;
    double value;
    Ease   easeToNext = Ease::InOutCubic;
};

class Track {
public:
    Track& key(double time, double value, Ease e = Ease::InOutCubic);
    double at(double time) const;
    double duration() const;

private:
    std::vector<Key> keys_;   // kept sorted by time
};

}  // namespace anim
