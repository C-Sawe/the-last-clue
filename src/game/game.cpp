// The playable prototype — Caleb (CP3, issue #47)
#include "game.h"

#include <cmath>

#include "animation/animation.h"
#include "animation/transitions.h"
#include "core/font.h"
#include "primitives/primitives.h"
#include "scenes/scenes.h"
#include "ui/text.h"

namespace game {

Game::Game() { board_.sync(st_); }

void Game::onEnter(Screen s) {
    if (s == Screen::Painting) painting_.enter();
    if (s == Screen::Board) board_.sync(st_);
}

// ------------------------------------------------------------------ update
void Game::update(double dt) {
    time_ += dt;
    st_.update(dt);
    if (title_) return;

    switch (st_.screen) {
        case Screen::Explore: explore_.update(dt, st_); break;
        case Screen::Painting: painting_.update(dt, st_); break;
        case Screen::Safe: safe_.update(dt); break;
        case Screen::Computer: laptop_.update(dt); break;
        default: break;
    }
    if (st_.screen != Screen::Explore) explore_.update(dt, st_);   // keeps animations (drawer) finishing

    // ---- react to puzzles being solved ----
    if (letter_.solved() && !st_.has("solved:letter")) {
        st_.set("solved:letter");
        st_.addEvidence(Evidence::TornLetter);
        letterTimer_ = 2.2;
    }
    if (letterTimer_ > 0 && (letterTimer_ -= dt) <= 0) {
        st_.go(Screen::Explore);
        st_.say("THE LETTER SAYS: THE CODE IS IN THE PAINTING");
    }
    if (const std::string m = laptop_.takeMessage(); !m.empty()) st_.say(m);
    if (laptop_.solved() && !st_.has("solved:code")) st_.set("solved:code");
    if (const std::string m = safe_.takeMessage(); !m.empty()) st_.say(m);
    if (safe_.doorOpen() && !st_.has("solved:safe")) {
        st_.set("solved:safe");
        st_.addEvidence(Evidence::Key);
        st_.addEvidence(Evidence::Fingerprint);
        st_.say("NEW EVIDENCE: DOOR KEY + FINGERPRINT");
    }

    if (fade_ < 1) fade_ = std::min(1.0, fade_ + dt / 0.35);
}

// ------------------------------------------------------------------ input
void Game::mouseMove(PointF p) {
    mouse_ = p;
    panel_.mouseMove(p);
    const PointF q = local(p);
    explore_.mouseMove(q);
    switch (st_.screen) {
        case Screen::Letter: letter_.mouseMove(q); break;
        case Screen::Safe: safe_.mouseMove(q); break;
        case Screen::Computer: laptop_.mouseMove(q); break;
        case Screen::Board: board_.mouseMove(q); break;
        default: break;
    }
}

void Game::mouseDown(PointF p, input::Button b) {
    if (title_) { title_ = false; return; }
    if (p.x < PanelW) {                                       // Case File panel
        switch (panel_.click(p, st_)) {
            case ui::CaseFileUI::Action::Board: st_.go(Screen::Board); break;
            case ui::CaseFileUI::Action::Deduction: st_.go(Screen::Deduction); break;
            case ui::CaseFileUI::Action::Back: st_.go(Screen::Explore); break;
            case ui::CaseFileUI::Action::None: break;
        }
        return;
    }
    const PointF q = local(p);
    switch (st_.screen) {
        case Screen::Explore: if (b == input::Button::Left) explore_.click(q, st_); break;
        case Screen::Letter:
            if (b == input::Button::Right) letter_.scroll(q, -1);   // right click rotates the other way
            else letter_.mouseDown(q);
            break;
        case Screen::Safe: safe_.mouseDown(q); break;
        case Screen::Computer: laptop_.mouseDown(q); break;
        case Screen::Board: board_.mouseDown(q, st_); break;
        case Screen::Painting: if (painting_.revealed()) st_.go(Screen::Explore); break;
        case Screen::Deduction: st_.go(Screen::Explore); break;
    }
}

void Game::mouseUp(PointF p, input::Button) {
    const PointF q = local(p);
    if (st_.screen == Screen::Letter) letter_.mouseUp(q);
    if (st_.screen == Screen::Safe) safe_.mouseUp(q);
    if (st_.screen == Screen::Board) board_.mouseUp(q);
}

void Game::scroll(PointF p, double dir) {
    if (st_.screen == Screen::Letter) letter_.scroll(local(p), dir);
}

void Game::keyDown(input::Key k) {
    if (title_) { if (k == input::Key::Enter || k == input::Key::Space) title_ = false; return; }
    if (k == input::Key::Escape) { if (st_.screen != Screen::Explore) st_.go(Screen::Explore); return; }
    switch (st_.screen) {
        case Screen::Letter: letter_.key(k, local(mouse_)); break;
        case Screen::Safe: safe_.key(k); break;
        case Screen::Computer: laptop_.key(k); break;
        default: break;
    }
}

void Game::character(unsigned cp) {
    if (!title_ && st_.screen == Screen::Computer) laptop_.type(cp);
}

bool Game::hoverOutline(Polygon& out) const {
    if (title_ || st_.screen != Screen::Explore || fade_ < 1) return false;
    const Hotspot* h = explore_.hovered();
    if (!h) return false;
    out = apply(translate(PanelW, 0), h->area);
    return true;
}

// ------------------------------------------------------------------ render
void Game::renderView(Canvas& v) {
    switch (st_.screen) {
        case Screen::Explore: explore_.render(v, st_); break;
        case Screen::Letter: letter_.render(v); break;
        case Screen::Painting: painting_.render(v); break;
        case Screen::Computer: laptop_.render(v); break;
        case Screen::Safe: safe_.render(v); break;
        case Screen::Board: board_.render(v, st_); break;
        case Screen::Deduction: renderDeduction(v); break;
    }
}

void Game::render(Canvas& c) {
    if (st_.screen != lastScreen_) {                          // screen changed: start a cross-fade
        fromView_ = lastView_;
        fade_ = 0;
        onEnter(st_.screen);
        lastScreen_ = st_.screen;
    }
    Canvas panel(PanelW, H), view(800, 500);
    panel_.render(panel, st_);
    renderView(view);
    lastView_ = view;
    if (fade_ < 1) {
        Canvas mixed(800, 500);
        anim::crossFade(mixed, fromView_, view, anim::ease(anim::Ease::InOutCubic, fade_));
        view = mixed;
    }
    ui::toastBar(view, 0, 470, 800, st_.toast);
    c.blit(panel, 0, 0);
    c.blit(view, PanelW, 0);

    Polygon outline;
    if (drawHoverInCanvas && hoverOutline(outline)) {
        drawPolygonOutline(c, outline, SuspectColour::Highlight);
        drawPolygonOutline(c, apply(translate(1, 1), outline), SuspectColour::Highlight);
    }
    if (title_) renderTitle(c);
}

void Game::renderDeduction(Canvas& v) const {
    v.clear(Color{24, 22, 30});
    drawText(v, 400 - textWidth("FINAL DEDUCTION", 4) / 2, 40, "FINAL DEDUCTION", Palette::White, 4);
    const char* qs[] = {"WHO?", "HOW?", "WHY?"};
    for (int i = 0; i < 3; ++i) {
        const Polygon card = makeRect(60 + i * 240, 120, 200, 200);
        scanlineFill(v, card, Color{48, 46, 60});
        drawPolygonOutline(v, card, Color{90, 88, 110});
        drawText(v, 160 + i * 240 - textWidth(qs[i], 4) / 2, 200, qs[i], Color{150, 148, 170}, 4);
    }
    const std::string s = std::to_string(st_.evidence.size()) + " PIECES OF EVIDENCE, " + std::to_string(st_.links.size()) + " STRINGS ON THE BOARD";
    drawText(v, 400 - textWidth(s, 2) / 2, 350, s, Color{200, 200, 210}, 2);
    ui::textBox(v, 100, 390, 600, 60, "THE DEDUCTION SCREEN (WHO / HOW / WHY) ARRIVES IN THE FINAL BUILD. CLICK TO RETURN TO THE STUDY.",
                Color{255, 214, 60}, 1);
}

void Game::renderTitle(Canvas& c) const {
    for (int y = 0; y < c.height(); ++y)
        for (int x = 0; x < c.width(); ++x) c.setPixel(x, y, anim::lerp(c.getPixel(x, y), Color{8, 8, 12}, 0.78));
    drawText(c, W / 2 - textWidth("THE LAST CLUE", 7) / 2, 120, "THE LAST CLUE", Palette::White, 7);
    drawText(c, W / 2 - textWidth("A DETECTIVE STORY DRAWN BY OUR OWN ALGORITHMS", 2) / 2, 190,
             "A DETECTIVE STORY DRAWN BY OUR OWN ALGORITHMS", Color{200, 200, 210}, 2);
    if (std::fmod(time_, 1.2) < 0.8)
        drawText(c, W / 2 - textWidth("CLICK TO BEGIN", 3) / 2, 270, "CLICK TO BEGIN", Color{255, 214, 60}, 3);
    const char* help[] = {"CLICK OBJECTS TO INVESTIGATE.  ESC RETURNS TO THE STUDY.",
                          "LETTER: DRAG PIECES, SCROLL / R / RIGHT-CLICK TO ROTATE.",
                          "SAFE: DRAG THE DIAL, ENTER LOCKS A NUMBER.  LAPTOP: TYPE, F1 OR RUN."};
    for (int i = 0; i < 3; ++i) drawText(c, W / 2 - textWidth(help[i]) / 2, 360 + i * 16, help[i], Color{170, 172, 186});
}

}  // namespace game
