// The playable crime scene — Alvin (CP3, issue #44)
#include "explore.h"

#include <algorithm>
#include <cmath>

#include "animation/animation.h"
#include "core/font.h"
#include "primitives/primitives.h"
#include "scenes/scene_anim.h"
#include "scenes/scenes.h"
#include "transform/transform2d.h"

namespace game {

namespace {
const double PI = 3.14159265358979;

// The laptop on the filing cabinet: base + tilted screen (polygons), screen
// colour shows the alarm state.
void drawLaptop(Canvas& c, bool alarmOff) {
    const Polygon screen{{{551, 132}, {603, 128}, {606, 168}, {548, 169}}};
    const Polygon base{{{540, 169}, {612, 169}, {618, 177}, {534, 177}}};
    scanlineFill(c, screen, Color{30, 30, 36});
    drawPolygonOutline(c, screen, Palette::Black);
    scanlineFill(c, Polygon{{{555, 136}, {599, 133}, {601, 164}, {553, 165}}}, alarmOff ? Color{60, 170, 90} : Color{190, 50, 50});
    drawText(c, 559, 146, alarmOff ? "OK" : "ALARM", Palette::White);
    scanlineFill(c, base, Color{150, 152, 160});
    drawPolygonOutline(c, base, Palette::Black);
}
}  // namespace

ExploreScreen::ExploreScreen() {
    const Polygon papers = apply(rotateAbout(-0.12, {385, 242}), makeRect(354, 230, 62, 24));
    spots_ = {
        {"door", makeRect(640, 120, 92, 212), "DOOR"},
        {"clock", makeRegularPolygon({470, 100}, 32, 16), "CLOCK"},
        {"painting", makeRect(300, 60, 100, 80), "PAINTING"},
        {"safe", makeRect(35, 235, 115, 110), "SAFE"},
        {"papers", papers, "PAPERS"},
        {"drawer", makeRect(400, 262, 108, 38), "DESK DRAWER"},
        {"laptop", Polygon{{{534, 126}, {608, 126}, {618, 178}, {532, 178}}}, "LAPTOP"},
    };
}

double ExploreScreen::standX(const std::string& id) const {
    if (id == "safe") return 175;
    if (id == "drawer") return 455;
    if (id == "painting") return 350;
    if (id == "laptop") return 575;
    if (id == "door") return 690;
    if (id == "clock") return 470;
    return 385;                                                     // papers
}

void ExploreScreen::mouseMove(PointF p) { mouse_ = p; }

void ExploreScreen::click(PointF p, GameState& st) {
    if (busy() || openingDrawer_) return;
    const Hotspot* h = pick(spots_, p);
    if (!h) return;
    pending_ = h->id;
    targetX_ = standX(h->id);
    if (std::fabs(targetX_ - x_) < 2) arrive(st);
}

void ExploreScreen::update(double dt, GameState& st) {
    if (!pending_.empty()) {                                        // walking: translation over time
        const double speed = 260;
        const double step = std::min(std::fabs(targetX_ - x_), speed * dt);
        x_ += (targetX_ > x_ ? step : -step);
        phase_ += dt * 12;
        if (std::fabs(targetX_ - x_) < 0.5) { x_ = targetX_; arrive(st); }
    }
    if (st.has("open:drawer") && drawerT_ < 1) drawerT_ = std::min(1.0, drawerT_ + dt / 0.6);
    if (openingDrawer_ && drawerT_ >= 1) {                          // drawer fully out -> close-up
        openingDrawer_ = false;
        st.go(Screen::Letter);
    }
    if (st.has("open:door") && doorT_ < 1) doorT_ = std::min(1.0, doorT_ + dt / 0.9);
}

// What each object does when the detective reaches it.
void ExploreScreen::arrive(GameState& st) {
    const std::string id = pending_;
    pending_.clear();
    if (id == "drawer") {
        if (st.has("solved:letter")) { st.say("ONLY DUST LEFT IN THE DRAWER"); return; }
        if (!st.has("open:drawer")) { st.set("open:drawer"); st.say("THE DRAWER SLIDES OPEN... TORN PAPER INSIDE"); openingDrawer_ = true; }
        else st.go(Screen::Letter);
    } else if (id == "safe") {
        if (st.has("solved:safe")) st.say("THE SAFE IS EMPTY NOW");
        else st.go(Screen::Safe);
    } else if (id == "painting") {
        st.go(Screen::Painting);
    } else if (id == "laptop") {
        st.go(Screen::Computer);
    } else if (id == "door") {
        if (st.hasEvidence(Evidence::Key)) { st.set("open:door"); st.say("THE KEY FITS. THE OLD MILL AWAITS (FINAL BUILD)"); }
        else st.say("LOCKED. IT NEEDS A KEY");
    } else if (id == "clock") {
        st.say("STOPPED AT 9:00... THE TIME ON THE NOTE?");
    } else {
        st.say("BILLS AND RECEIPTS. NOTHING USEFUL");
    }
}

void ExploreScreen::render(Canvas& c, const GameState& st) const {
    SceneState scene;
    // (closed must be exactly 0: the scene shows the "interactive" outline only then)
    scene.deskDrawer = drawerT_ <= 0 ? 0.0 : anim::ease(anim::Ease::OutBack, drawerT_);
    scene.door = doorT_ <= 0 ? 0.0 : anim::ease(anim::Ease::InOutCubic, doorT_);
    renderCrimeSceneState(c, scene);
    drawLaptop(c, st.has("solved:code"));
    drawDetective(c, {x_, 462}, phase_, !pending_.empty());

    if (const Hotspot* h = hovered()) {                             // name tag above the cursor
        const int w = textWidth(h->label, 2) + 16;
        const int x = std::min(static_cast<int>(mouse_.x) - w / 2, c.width() - w - 4), y = static_cast<int>(mouse_.y) - 34;
        const Polygon tag = makeRect(std::max(4, x), std::max(4, y), w, 24);
        scanlineFill(c, tag, Color{20, 20, 26});
        drawPolygonOutline(c, tag, SuspectColour::Highlight);
        drawText(c, std::max(4, x) + 8, std::max(4, y) + 5, h->label, Palette::White, 2);
    }
}

// =============================== PaintingView ===============================
void PaintingView::update(double dt, GameState& st) {
    t_ = std::min(1.0, t_ + dt / 1.6);
    if (t_ >= 1 && !st.hasEvidence(Evidence::SafeCode)) {
        st.set("seen:code");
        st.addEvidence(Evidence::SafeCode);
    }
}

void PaintingView::render(Canvas& c) const {
    // Wall and the hidden recess behind the painting
    c.clear(Color{70, 78, 96});
    for (int x = 20; x < c.width(); x += 40) drawLineBresenham(c, {x, 0}, {x, c.height() - 1}, Color{78, 87, 106});
    const Polygon recess = makeRect(230, 110, 340, 260);
    scanlineFill(c, recess, Color{38, 40, 50});
    drawPolygonOutline(c, recess, Palette::Black);
    drawText(c, 400 - textWidth("37 - 12 - 85", 4) / 2, 150, "37 - 12 - 85", Color{235, 235, 225}, 4);
    const Polygon star = makeStar({400, 280}, 62, 24, 5);           // the victim's symbol
    for (size_t i = 0; i < star.vertices.size(); i += 2) {          // drawn as the 5 crossing strokes
        const PointF a = star.vertices[i], b = star.vertices[(i + 4) % star.vertices.size()];
        drawThickLine(c, {static_cast<int>(a.x), static_cast<int>(a.y)}, {static_cast<int>(b.x), static_cast<int>(b.y)}, 3, Color{235, 235, 225});
    }
    drawText(c, 250, 352, "HER SYMBOL", Color{200, 200, 190});

    // The painting, drawn once on its own canvas, then swung about the nail
    Canvas art(380, 290);
    art.clear(Color{190, 150, 60});
    scanlineFill(art, makeRect(14, 14, 352, 262), Color{120, 160, 190});
    scanlineFill(art, Polygon{{{14, 276}, {130, 120}, {230, 276}}}, Color{80, 100, 80});
    scanlineFill(art, Polygon{{{170, 276}, {280, 140}, {366, 276}}}, Color{60, 85, 65});
    fillCircle(art, {300, 70}, 24, Palette::Yellow);
    drawPolygonOutline(art, makeRect(0, 0, 380, 290), Palette::Black);

    const PointF nail{210, 95}, artPos{210, 95};                    // top-left corner hangs on the nail
    const double angle = anim::ease(anim::Ease::InOutCubic, t_) * 1.25;   // swings ~70 degrees
    const Mat3 M = rotateAbout(angle, nail) * translate(artPos.x, artPos.y);
    const Mat3 back = inverse(M);
    const Polygon outline = apply(M, makeRect(0, 0, 380, 290));
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (const auto& v : outline.vertices) {
        minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y); maxY = std::max(maxY, v.y);
    }
    scanlineFill(c, apply(translate(6, 8), outline), Color{40, 44, 56});   // shadow
    for (int y = std::max(0, static_cast<int>(minY)); y <= std::min(c.height() - 1, static_cast<int>(maxY)); ++y)
        for (int x = std::max(0, static_cast<int>(minX)); x <= std::min(c.width() - 1, static_cast<int>(maxX)); ++x) {
            const PointF s = apply(back, PointF{x + 0.5, y + 0.5});
            if (art.inBounds(static_cast<int>(s.x), static_cast<int>(s.y)) && s.x >= 0 && s.y >= 0)
                c.setPixel(x, y, art.getPixel(static_cast<int>(s.x), static_cast<int>(s.y)));
        }
    fillCircle(c, {static_cast<int>(nail.x), static_cast<int>(nail.y)}, 5, Color{180, 180, 190});
    drawText(c, 20, 470, t_ < 1 ? "THE PAINTING SWINGS ASIDE..." : "THE SAFE CODE - AND HER SYMBOL", Palette::White, 2);
}

}  // namespace game
