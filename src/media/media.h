#pragma once
// Meme branch: pictures, video clips and sound.
//
// Everything is optional. Files live in assets/memes/ (made by
// tools/prepare_memes.sh); a missing file simply means that effect is skipped,
// and if no sound device can be opened the game stays silent.
//
//   images  assets/memes/<name>.jpg            -> media::image("mem1")
//   videos  assets/memes/<name>/0001.jpg ...   -> media::video("lost1")
//           + audio.mp3 + fps.txt
//   sounds  assets/memes/<name>.mp3            -> media::playSound("yaah")
//   effects synthesised in code                -> media::sfx(media::Sfx::Click)
#include <memory>
#include <string>
#include <vector>

#include "core/canvas.h"

namespace media {

// Finds assets/memes (LASTCLUE_ASSETS, ./assets/memes or ../assets/memes) and
// opens the sound device. Safe to call more than once.
void init();
bool assetsFound();
std::string assetDir();
void setMuted(bool muted);          // e.g. for the scripted playthrough

// ---- pictures -----------------------------------------------------------------
const Canvas* image(const std::string& name);        // cached; nullptr if missing

// Draws `src` scaled to cover the box (cropping the overflow), by inverse mapping:
// every destination pixel is mapped back into the source. `fade` 0..1 washes the
// picture out (desaturate + lift the blacks) for the "faded meme" look.
void drawCover(Canvas& dst, const Canvas& src, double x, double y, double w, double h, double fade = 0);
// Same, but fits the whole picture inside the box (letterboxed), and blends it over
// what is already there with `alpha`.
void drawFit(Canvas& dst, const Canvas& src, double x, double y, double w, double h, double fade, double alpha);

// ---- video clips ----------------------------------------------------------------
class Video {
public:
    explicit Video(const std::string& folder);
    bool ok() const { return !frames_.empty(); }
    double duration() const { return frames_.size() / fps_; }
    const Canvas& frameAt(double seconds);             // decodes on demand, keeps the last one
    const std::string& audioPath() const { return audio_; }

private:
    std::vector<std::vector<unsigned char>> frames_;   // the JPEG bytes of every frame
    double fps_ = 12;
    int current_ = -1;
    Canvas frame_;
    std::string audio_;
};
std::shared_ptr<Video> video(const std::string& name);   // cached; nullptr if missing

// ---- sound --------------------------------------------------------------------------
void playSound(const std::string& name);              // assets/memes/<name>.mp3, if present
bool soundExists(const std::string& name);
void playFile(const std::string& path);
void stopAll();                                        // stops files (videos, songs), not effects

enum class Sfx { Click, Tick, Snap, Drawer, Door, SafeOpen, Evidence, Success, Wrong, Type, Whoosh, Stamp, Rumble, Scream };
void sfx(Sfx s);                                       // short effect, synthesised in code

}  // namespace media
