#pragma once
// Meme branch: full-screen meme moments played over the game view.
//
// A queue of clips. Each clip is a picture or a video (from media/), shown with a
// washed-out "faded" look, and may start a sound. While a clip plays the game
// pauses, and a click or a key skips to the next clip.
//   - Picture: fades in, holds, fades out over the game (alpha blending).
//   - Video:   the game behind it darkens; the clip is scaled to the view by
//              inverse mapping; optional shake at the start (jump scare).
//   - Scare:   clip "@scare" is a jump scare drawn entirely in code.
//   - Wasted:  after a moment the whole frame turns grey and a big WASTED with a
//              caption drops in, GTA style.
#include <deque>
#include <memory>
#include <string>

#include "core/canvas.h"
#include "media/media.h"

namespace game {

struct MemeClip {
    std::string name;            // media name: mem4, lost1, ...
    bool video = false;
    double seconds = 0;          // 0 = the whole video
    std::string sound;           // extra sound to start with it (e.g. "yaah")
    bool wasted = false;
    bool shake = false;          // jump scare: shake + flash at the start
    std::string caption;         // small text under the picture
};

class MemeLayer {
public:
    void setEnabled(bool on) { enabled_ = on; }
    bool enabled() const { return enabled_; }
    void queue(const MemeClip& c);       // ignored when disabled or the file is missing
    bool active() const { return !clips_.empty(); }
    void update(double dt);
    void skip();                         // click / key: end the current clip
    void render(Canvas& view);           // 800 x 500, drawn over the game view
    void clear();

private:
    std::deque<MemeClip> clips_;
    double t_ = 0;
    bool started_ = false;
    bool enabled_ = true;
    std::shared_ptr<media::Video> video_;
    void start();
    double length() const;
    // the door's jump scare, drawn in code (clip name "@scare")
    static constexpr double ScareHit = 0.65;
    bool screamed_ = false;
    void renderScare(Canvas& v);
};

}  // namespace game
