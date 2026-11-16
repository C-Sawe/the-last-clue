// Meme branch: pictures, video clips and sound.
#include "media.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>

#include "miniaudio.h"
#include "stb_image.h"

namespace media {

namespace {
std::string g_dir;
bool g_searched = false;
bool g_muted = false;

bool fileExists(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return f.good();
}

std::vector<unsigned char> readFile(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

bool decode(const unsigned char* data, size_t size, Canvas& out) {
    int w = 0, h = 0, n = 0;
    unsigned char* px = stbi_load_from_memory(data, static_cast<int>(size), &w, &h, &n, 3);
    if (!px) return false;
    out = Canvas(w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            const unsigned char* p = px + 3 * (y * w + x);
            out.setPixel(x, y, Color{p[0], p[1], p[2]});
        }
    stbi_image_free(px);
    return true;
}

// The faded look: pull the colour towards grey (desaturate), lift the blacks
// and warm it slightly, like an old photo left in the sun.
Color fadeColor(Color c, double fade) {
    if (fade <= 0) return c;
    const double grey = 0.30 * c.r + 0.59 * c.g + 0.11 * c.b;
    auto ch = [&](double v, double warm) {
        v = v + (grey - v) * 0.55 * fade;                 // desaturate
        v = v * (1 - 0.30 * fade) + (60 + warm) * fade * 0.55;   // lift the blacks, wash out
        return static_cast<uint8_t>(std::clamp(v, 0.0, 255.0));
    };
    return Color{ch(c.r, 25), ch(c.g, 15), ch(c.b, 0)};
}

Color mix(Color a, Color b, double t) {
    return Color{static_cast<uint8_t>(a.r + (b.r - a.r) * t), static_cast<uint8_t>(a.g + (b.g - a.g) * t),
                 static_cast<uint8_t>(a.b + (b.b - a.b) * t)};
}

// ------------------------------------------------------------------ audio
struct Audio {
    ma_engine engine;
    bool ready = false;
    struct Playing {
        ma_sound sound;
        ma_audio_buffer* buffer = nullptr;   // only for synthesised effects
        bool isFile = false;
    };
    std::vector<Playing*> playing;
    std::mutex lock;

    void tidy() {                            // free sounds that have finished
        for (auto it = playing.begin(); it != playing.end();) {
            if (!ma_sound_is_playing(&(*it)->sound) && ma_sound_at_end(&(*it)->sound)) {
                release(*it);
                it = playing.erase(it);
            } else {
                ++it;
            }
        }
    }
    void release(Playing* p) {
        ma_sound_uninit(&p->sound);
        if (p->buffer) ma_audio_buffer_uninit_and_free(p->buffer);
        delete p;
    }
};
Audio g_audio;

void startAudio() {
    static bool tried = false;
    if (tried || std::getenv("LASTCLUE_NO_AUDIO")) return;
    tried = true;
    g_audio.ready = ma_engine_init(nullptr, &g_audio.engine) == MA_SUCCESS;
}

// ------------------------------------------------------------- synthesis
// Each effect is built sample by sample: oscillators (sine, square, saw),
// noise, an exponential decay envelope and a one-pole low-pass filter.
const int RATE = 44100;
const double TAU = 6.283185307179586;

struct Synth {
    std::vector<float> s;
    unsigned seed = 1234;
    double noise() { seed = seed * 1664525u + 1013904223u; return ((seed >> 9) / 4194304.0) * 2 - 1; }
    void ensure(double seconds) { s.resize(std::max(s.size(), static_cast<size_t>(seconds * RATE))); }
    // a tone gliding from f0 to f1 Hz, starting at `at` seconds
    void tone(double at, double len, double f0, double f1, double vol, double decay, int wave = 0) {
        ensure(at + len);
        double phase = 0;
        for (int i = 0; i < static_cast<int>(len * RATE); ++i) {
            const double t = static_cast<double>(i) / RATE, f = f0 + (f1 - f0) * (t / len);
            phase += f / RATE;
            double v = std::sin(TAU * phase);
            if (wave == 1) v = v > 0 ? 1 : -1;                         // square
            if (wave == 2) v = 2 * (phase - std::floor(phase)) - 1;     // saw
            const double env = std::exp(-t * decay) * std::min(1.0, t * 400);
            s[static_cast<size_t>(at * RATE) + i] += static_cast<float>(v * vol * env);
        }
    }
    // filtered noise; `cut` 0..1 is the low-pass amount
    void hiss(double at, double len, double vol, double decay, double cut0, double cut1) {
        ensure(at + len);
        double y = 0;
        for (int i = 0; i < static_cast<int>(len * RATE); ++i) {
            const double t = static_cast<double>(i) / RATE, cut = cut0 + (cut1 - cut0) * (t / len);
            y += (noise() - y) * cut;
            const double env = std::exp(-t * decay) * std::min(1.0, t * 300);
            s[static_cast<size_t>(at * RATE) + i] += static_cast<float>(y * vol * env);
        }
    }
};

std::vector<float> makeSfx(Sfx kind) {
    Synth x;
    switch (kind) {
        case Sfx::Click: x.hiss(0, 0.03, 0.5, 120, 0.6, 0.6); x.tone(0, 0.03, 1800, 1200, 0.15, 150); break;
        case Sfx::Type: x.hiss(0, 0.02, 0.25, 200, 0.7, 0.7); break;
        case Sfx::Tick: x.tone(0, 0.025, 3200, 3000, 0.25, 180); x.hiss(0, 0.01, 0.2, 300, 0.9, 0.9); break;
        case Sfx::Snap: x.tone(0, 0.06, 900, 1400, 0.3, 40); x.hiss(0, 0.03, 0.3, 120, 0.8, 0.8); break;
        case Sfx::Drawer: x.hiss(0, 0.45, 0.6, 4, 0.08, 0.02); x.tone(0.38, 0.12, 90, 60, 0.4, 25); break;
        case Sfx::Door:                                             // a creak: a wobbling saw through a filter
            x.ensure(1.3);
            for (int i = 0; i < static_cast<int>(1.2 * RATE); ++i) {
                const double t = static_cast<double>(i) / RATE;
                const double f = 110 + 40 * std::sin(TAU * 3.1 * t) + 30 * t;
                static double ph = 0, y = 0;
                if (i == 0) { ph = 0; y = 0; }
                ph += f / RATE;
                y += ((2 * (ph - std::floor(ph)) - 1) - y) * 0.12;
                x.s[static_cast<size_t>(i)] += static_cast<float>(y * 0.35 * std::exp(-t * 1.2) * std::min(1.0, t * 20));
            }
            break;
        case Sfx::SafeOpen: x.tone(0, 0.35, 70, 45, 0.6, 9); x.tone(0.05, 0.6, 880, 870, 0.15, 6); x.hiss(0, 0.08, 0.4, 40, 0.5, 0.2); break;
        case Sfx::Evidence: x.tone(0, 0.12, 523, 523, 0.25, 12); x.tone(0.09, 0.12, 659, 659, 0.25, 12); x.tone(0.18, 0.3, 784, 784, 0.25, 7); break;
        case Sfx::Success:
            for (int k = 0; k < 4; ++k) x.tone(0.11 * k, 0.5, 523 * std::pow(1.26, k), 523 * std::pow(1.26, k), 0.18, 5);
            break;
        case Sfx::Wrong: x.tone(0, 0.18, 150, 140, 0.25, 4, 1); x.tone(0.2, 0.3, 110, 100, 0.25, 4, 1); break;
        case Sfx::Whoosh: x.hiss(0, 0.35, 0.5, 6, 0.02, 0.25); break;
        case Sfx::Stamp: x.tone(0, 0.25, 90, 50, 0.7, 14); x.hiss(0, 0.1, 0.6, 35, 0.4, 0.1); break;
        case Sfx::Rumble: x.tone(0, 0.7, 42, 38, 0.6, 0.8); x.hiss(0, 0.7, 0.35, 0.5, 0.01, 0.03); break;
        case Sfx::Scream:                                           // a shriek: detuned saws sliding down + harsh noise
            x.tone(0, 1.3, 1450, 700, 0.30, 1.4, 2);
            x.tone(0, 1.3, 1530, 760, 0.25, 1.4, 2);
            x.tone(0, 1.3, 980, 480, 0.25, 1.6, 1);
            x.hiss(0, 1.3, 0.55, 1.5, 0.9, 0.4);
            x.tone(0, 0.5, 70, 40, 0.8, 4);                         // the hit
            for (size_t i = 0; i < x.s.size(); ++i)                 // tremble: amplitude wobble at 18 Hz
                x.s[i] *= static_cast<float>(0.75 + 0.25 * std::sin(TAU * 18 * static_cast<double>(i) / RATE));
            break;
    }
    for (auto& v : x.s) v = std::clamp(v, -1.0f, 1.0f);
    return x.s;
}
}  // namespace

// ================================================================== setup
void init() {
    if (!g_searched) {
        g_searched = true;
        std::vector<std::string> tries;
        if (const char* env = std::getenv("LASTCLUE_ASSETS")) tries.push_back(env);
        tries.push_back("assets/memes");
        tries.push_back("../assets/memes");
        tries.push_back("../../assets/memes");
        for (const auto& d : tries)
            if (fileExists(d + "/mem1.jpg") || fileExists(d + "/lost1/fps.txt") || fileExists(d + "/README.txt")) { g_dir = d; break; }
    }
}

bool assetsFound() { return !g_dir.empty(); }
std::string assetDir() { return g_dir; }
void setMuted(bool m) { g_muted = m; }

// ================================================================= images
const Canvas* image(const std::string& name) {
    static std::map<std::string, std::unique_ptr<Canvas>> cache;
    auto it = cache.find(name);
    if (it != cache.end()) return it->second.get();
    std::unique_ptr<Canvas> c;
    if (!g_dir.empty())
        for (const char* ext : {".jpg", ".jpeg", ".png"}) {
            const std::string p = g_dir + "/" + name + ext;
            if (!fileExists(p)) continue;
            const auto bytes = readFile(p);
            Canvas tmp(1, 1);
            if (decode(bytes.data(), bytes.size(), tmp)) c = std::make_unique<Canvas>(tmp);
            break;
        }
    const Canvas* out = c.get();
    cache[name] = std::move(c);
    return out;
}

void drawCover(Canvas& dst, const Canvas& src, double x, double y, double w, double h, double fade) {
    const double k = std::max(w / src.width(), h / src.height());          // cover: crop the overflow
    const double ox = (src.width() - w / k) / 2, oy = (src.height() - h / k) / 2;
    for (int py = std::max(0, static_cast<int>(y)); py < std::min(dst.height(), static_cast<int>(y + h)); ++py)
        for (int px = std::max(0, static_cast<int>(x)); px < std::min(dst.width(), static_cast<int>(x + w)); ++px) {
            const int sx = std::clamp(static_cast<int>(ox + (px + 0.5 - x) / k), 0, src.width() - 1);
            const int sy = std::clamp(static_cast<int>(oy + (py + 0.5 - y) / k), 0, src.height() - 1);
            dst.setPixel(px, py, fadeColor(src.getPixel(sx, sy), fade));
        }
}

void drawFit(Canvas& dst, const Canvas& src, double x, double y, double w, double h, double fade, double alpha) {
    const double k = std::min(w / src.width(), h / src.height());          // fit: show all of it
    const double dw = src.width() * k, dh = src.height() * k;
    const double x0 = x + (w - dw) / 2, y0 = y + (h - dh) / 2;
    for (int py = std::max(0, static_cast<int>(y0)); py < std::min(dst.height(), static_cast<int>(y0 + dh)); ++py)
        for (int px = std::max(0, static_cast<int>(x0)); px < std::min(dst.width(), static_cast<int>(x0 + dw)); ++px) {
            const int sx = std::clamp(static_cast<int>((px + 0.5 - x0) / k), 0, src.width() - 1);
            const int sy = std::clamp(static_cast<int>((py + 0.5 - y0) / k), 0, src.height() - 1);
            const Color c = fadeColor(src.getPixel(sx, sy), fade);
            dst.setPixel(px, py, alpha >= 1 ? c : mix(dst.getPixel(px, py), c, alpha));
        }
}

// ================================================================== video
Video::Video(const std::string& folder) : frame_(1, 1) {
    for (int i = 1;; ++i) {
        char name[16];
        std::snprintf(name, sizeof name, "/%04d.jpg", i);
        const std::string p = folder + name;
        if (!fileExists(p)) break;
        frames_.push_back(readFile(p));
    }
    std::ifstream fps(folder + "/fps.txt");
    if (fps) fps >> fps_;
    if (fps_ <= 0) fps_ = 12;
    if (fileExists(folder + "/audio.mp3")) audio_ = folder + "/audio.mp3";
}

const Canvas& Video::frameAt(double seconds) {
    if (frames_.empty()) return frame_;
    const int i = std::clamp(static_cast<int>(seconds * fps_), 0, static_cast<int>(frames_.size()) - 1);
    if (i != current_ && decode(frames_[static_cast<size_t>(i)].data(), frames_[static_cast<size_t>(i)].size(), frame_)) current_ = i;
    return frame_;
}

std::shared_ptr<Video> video(const std::string& name) {
    static std::map<std::string, std::shared_ptr<Video>> cache;
    auto it = cache.find(name);
    if (it != cache.end()) return it->second;
    std::shared_ptr<Video> v;
    if (!g_dir.empty() && fileExists(g_dir + "/" + name + "/0001.jpg")) {
        v = std::make_shared<Video>(g_dir + "/" + name);
        if (!v->ok()) v.reset();
    }
    cache[name] = v;
    return v;
}

// ================================================================== sound
void playFile(const std::string& path) {
    if (g_muted || path.empty()) return;
    startAudio();                                   // the sound device opens on the first sound
    if (!g_audio.ready) return;
    std::lock_guard<std::mutex> l(g_audio.lock);
    g_audio.tidy();
    auto* p = new Audio::Playing();
    p->isFile = true;
    if (ma_sound_init_from_file(&g_audio.engine, path.c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr, &p->sound) != MA_SUCCESS) {
        delete p;
        return;
    }
    ma_sound_start(&p->sound);
    g_audio.playing.push_back(p);
}

bool soundExists(const std::string& name) {
    return !g_dir.empty() && (fileExists(g_dir + "/" + name + ".mp3") || fileExists(g_dir + "/" + name + ".wav"));
}

void playSound(const std::string& name) {
    if (g_dir.empty()) return;
    for (const char* ext : {".mp3", ".wav"})
        if (fileExists(g_dir + "/" + name + ext)) { playFile(g_dir + "/" + name + ext); return; }
}

void stopAll() {
    if (!g_audio.ready) return;
    std::lock_guard<std::mutex> l(g_audio.lock);
    for (auto it = g_audio.playing.begin(); it != g_audio.playing.end();) {
        if ((*it)->isFile) {
            ma_sound_stop(&(*it)->sound);
            g_audio.release(*it);
            it = g_audio.playing.erase(it);
        } else {
            ++it;
        }
    }
}

void sfx(Sfx kind) {
    if (g_muted) return;
    startAudio();
    if (!g_audio.ready) return;
    static std::map<int, std::vector<float>> bank;                 // each effect is synthesised once
    auto& pcm = bank[static_cast<int>(kind)];
    if (pcm.empty()) pcm = makeSfx(kind);
    std::lock_guard<std::mutex> l(g_audio.lock);
    g_audio.tidy();
    auto* p = new Audio::Playing();
    ma_audio_buffer_config cfg = ma_audio_buffer_config_init(ma_format_f32, 1, pcm.size(), pcm.data(), nullptr);
    cfg.sampleRate = RATE;
    if (ma_audio_buffer_alloc_and_init(&cfg, &p->buffer) != MA_SUCCESS) { delete p; return; }
    if (ma_sound_init_from_data_source(&g_audio.engine, p->buffer, 0, nullptr, &p->sound) != MA_SUCCESS) {
        ma_audio_buffer_uninit_and_free(p->buffer);
        delete p;
        return;
    }
    ma_sound_start(&p->sound);
    g_audio.playing.push_back(p);
}

}  // namespace media
