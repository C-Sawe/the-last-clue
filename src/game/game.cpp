// The game — Caleb (CP3 issue #47, final build issue #54)
#include "game.h"

#include <algorithm>
#include <cmath>

#include "animation/animation.h"
#include "animation/transitions.h"
#include "core/font.h"
#include "deduction.h"
#include "media/media.h"
#include "primitives/primitives.h"
#include "scenes/scenes.h"
#include "ui/text.h"

namespace game {

Game::Game() {
    media::init();
    memesOn_ = media::assetsFound();
    memes_.setEnabled(memesOn_);
    board_.sync(st_);
}

void Game::setMemes(bool on) {
    memesOn_ = on;
    memes_.setEnabled(on);
    media::setMuted(!on);
    if (!on) memes_.clear();
}

void Game::restart() {
    const bool on = memesOn_;
    media::stopAll();
    *this = Game();
    setMemes(on);
    title_ = false;
}

void Game::onEnter(Screen s) {
    if (s == Screen::Painting) painting_.enter();
    if (s == Screen::Board) board_.sync(st_);
    if (s == Screen::Deduction) {
        deduction_.enter();
        if (!st_.has("meme:think")) {                         // first time at the deduction: "let me think"
            st_.set("meme:think");
            memes_.queue({"mem4", false, 3.5, "", false, false, "LET ME THINK..."});
        }
    }
    if (s != Screen::Outcome) media::sfx(media::Sfx::Whoosh);
    if (s == Screen::Outcome) stampTimer_ = 0.75;             // the stamp lands ~0.75 s into the verdict
}

// ------------------------------------------------------------------ update
void Game::update(double dt) {
    time_ += dt;
    st_.update(dt);
    panel_.update(dt, st_);
    if (title_) return;
    if (memes_.active()) { memes_.update(dt); return; }      // a meme is playing: the game waits
    if (stampTimer_ > 0 && st_.screen == Screen::Outcome && (stampTimer_ -= dt) <= 0) media::sfx(media::Sfx::Stamp);

    switch (st_.screen) {
        case Screen::Explore: explore_.update(dt, st_); break;
        case Screen::Painting: painting_.update(dt, st_); break;
        case Screen::Safe: safe_.update(dt); break;
        case Screen::Computer: laptop_.update(dt); break;
        case Screen::Panel: panelTerm_.update(dt); break;
        case Screen::Deduction: deduction_.update(dt); break;
        case Screen::Outcome: outcome_.update(dt); break;
        default: break;
    }
    if (st_.screen != Screen::Explore) explore_.update(dt, st_);   // keeps animations (drawer, door) finishing
    react(dt);
    if (fade_ < 1) fade_ = std::min(1.0, fade_ + dt / (st_.screen == Screen::Outcome ? 0.8 : 0.35));
}

// The rules that connect the screens: each puzzle, once solved, sets a flag,
// hands over evidence and may open the next part of the case.
void Game::react(double dt) {
    if (letter_.solved() && !st_.has("solved:letter")) {
        st_.set("solved:letter");
        st_.addEvidence(Evidence::TornLetter);
        letterTimer_ = 6.2;                                   // time to read it before going back
        media::sfx(media::Sfx::Success);
    }
    if (letterTimer_ > 0 && (letterTimer_ -= dt) <= 0) {
        st_.go(Screen::Explore);
        st_.say("THE LETTER SAYS: THE CODE IS IN THE PAINTING");
    }
    if (const std::string m = laptop_.takeMessage(); !m.empty()) st_.say(m);
    if (laptop_.solved(0) && !st_.has("solved:code")) {          // puzzle B: the safe's alarm
        st_.set("solved:code");
        laptop_.setLocked(1, false);
        st_.say("ALARM OFF. A NEW FILE APPEARED: MAIL.CLUE", 4);
        media::sfx(media::Sfx::Success);
    }
    if (laptop_.solved(1) && !st_.has("solved:email")) {         // puzzle C: the email
        st_.set("solved:email");
        st_.addEvidence(Evidence::Email);
    }
    if (const std::string m = panelTerm_.takeMessage(); !m.empty()) st_.say(m);
    if (panelTerm_.solved(0) && !st_.has("solved:wiring")) {     // puzzle A: the door's lock
        st_.set("solved:wiring");
        media::sfx(media::Sfx::Success);
        st_.say("DOOR LOCK POWERED - NOW IT NEEDS THE KEY", 4);
    }
    if (const std::string m = safe_.takeMessage(); !m.empty()) st_.say(m);
    if (safe_.doorOpen() && !st_.has("solved:safe")) {
        st_.set("solved:safe");
        media::sfx(media::Sfx::SafeOpen);
        st_.addEvidence(Evidence::Key);
        st_.addEvidence(Evidence::Fingerprint);
        st_.say("NEW EVIDENCE: DOOR KEY + FINGERPRINT");
    }
    if (const std::string m = prints_.takeMessage(); !m.empty()) st_.say(m, 4);
    if (prints_.solved() && !st_.has("solved:print")) { st_.set("solved:print"); media::sfx(media::Sfx::Success); }

    // Meme branch: sounds for things happening, and the door's jump scare.
    if (st_.has("open:drawer") && !st_.has("sfx:drawer")) { st_.set("sfx:drawer"); media::sfx(media::Sfx::Drawer); }
    if (st_.has("open:door") && !st_.has("sfx:door")) {
        st_.set("sfx:door");
        media::sfx(media::Sfx::Door);
        scareTimer_ = 1.1;                                    // let the creak play, then...
    }
    if (scareTimer_ > 0 && (scareTimer_ -= dt) <= 0) memes_.queue({"jumpscare", true, 0, "", false, true, ""});
    if (st_.evidence.size() > lastEvidence_) media::sfx(media::Sfx::Evidence);
    lastEvidence_ = st_.evidence.size();
    if (st_.links.size() > lastLinks_) media::sfx(media::Sfx::Snap);
    lastLinks_ = st_.links.size();
    if (st_.screen == Screen::Safe && safe_.number() != lastDial_) media::sfx(media::Sfx::Tick);
    lastDial_ = safe_.number();
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
        case Screen::Panel: panelTerm_.mouseMove(q); break;
        case Screen::Prints: prints_.mouseMove(q); break;
        case Screen::Deduction: deduction_.mouseMove(q); break;
        case Screen::Outcome: outcome_.mouseMove(q); break;
        default: break;
    }
}

void Game::mouseDown(PointF p, input::Button b) {
    if (title_) { title_ = false; media::sfx(media::Sfx::Click); return; }
    if (memes_.active()) { memes_.skip(); return; }           // a click skips the meme
    media::sfx(media::Sfx::Click);
    if (p.x < PanelW) {                                       // Case File panel
        if (st_.screen == Screen::Outcome) return;            // the verdict is in
        switch (panel_.click(p, st_)) {
            case ui::CaseFileUI::Action::Board: st_.go(Screen::Board); break;
            case ui::CaseFileUI::Action::Prints: st_.go(Screen::Prints); break;
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
        case Screen::Panel: panelTerm_.mouseDown(q); break;
        case Screen::Prints: prints_.mouseDown(q); break;
        case Screen::Deduction:
            if (deduction_.mouseDown(q) == ui::DeductionUI::Action::Accuse) {
                const Verdict v = judge(st_, deduction_.accusation());
                outcome_.show(v);
                st_.go(Screen::Outcome);
                if (v.solved) {                               // case solved: the song, and a "yaah"
                    memes_.queue({"songmem1", true, 0, "yaah", false, false, ""});
                } else {
                    media::sfx(media::Sfx::Wrong);
                    if (st_.accusations == 1) memes_.queue({"lost1", true, 0, "", false, false, ""});
                    if (st_.accusations == 1 || v.caseClosed) memes_.queue({"dofido1", true, 0, "", true, false, ""});
                }
            }
            break;
        case Screen::Outcome: outcomeAction(outcome_.mouseDown(q)); break;
    }
}

void Game::outcomeAction(ui::OutcomeUI::Action a) {
    if (a == ui::OutcomeUI::Action::PlayAgain) restart();
    if (a == ui::OutcomeUI::Action::Reopen) {
        deduction_.clear();
        st_.screen = Screen::Deduction;                       // straight back, no entry rules
    }
}

void Game::mouseUp(PointF p, input::Button) {
    const PointF q = local(p);
    if (st_.screen == Screen::Letter) letter_.mouseUp(q);
    if (st_.screen == Screen::Safe) safe_.mouseUp(q);
    if (st_.screen == Screen::Board) board_.mouseUp(q);
    if (st_.screen == Screen::Prints) prints_.mouseUp(q);
}

void Game::scroll(PointF p, double dir) {
    if (st_.screen == Screen::Letter) letter_.scroll(local(p), dir);
    if (st_.screen == Screen::Prints) prints_.scroll(local(p), dir);
}

void Game::keyDown(input::Key k) {
    if (title_) { if (k == input::Key::Enter || k == input::Key::Space) title_ = false; return; }
    if (memes_.active()) {                                    // Space / Enter / Esc skip the meme
        if (k == input::Key::Space || k == input::Key::Enter || k == input::Key::Escape) memes_.skip();
        return;
    }
    if (st_.screen == Screen::Outcome) { outcomeAction(outcome_.key(k)); return; }
    if (k == input::Key::Escape) { if (st_.screen != Screen::Explore) st_.go(Screen::Explore); return; }
    switch (st_.screen) {
        case Screen::Letter: letter_.key(k, local(mouse_)); break;
        case Screen::Safe: safe_.key(k); break;
        case Screen::Computer: laptop_.key(k); break;
        case Screen::Panel: panelTerm_.key(k); break;
        case Screen::Prints: prints_.key(k); break;
        default: break;
    }
}

void Game::character(unsigned cp) {
    if (title_ || memes_.active()) return;
    if (st_.screen == Screen::Computer || st_.screen == Screen::Panel) media::sfx(media::Sfx::Type);
    if (st_.screen == Screen::Computer) laptop_.type(cp);
    if (st_.screen == Screen::Panel) panelTerm_.type(cp);
}

bool Game::hoverOutline(Polygon& out) const {
    if (title_ || memes_.active() || st_.screen != Screen::Explore || fade_ < 1) return false;
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
        case Screen::Panel: panelTerm_.render(v); break;
        case Screen::Prints: prints_.render(v); break;
        case Screen::Deduction: deduction_.render(v, st_); break;
        case Screen::Outcome: outcome_.render(v); break;
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
        const double t = anim::ease(anim::Ease::InOutCubic, fade_);
        if (st_.screen == Screen::Outcome) anim::fadeThroughBlack(mixed, fromView_, view, t);   // the verdict: a beat of black
        else if (st_.screen == Screen::Deduction) anim::wipe(mixed, fromView_, view, t);
        else anim::crossFade(mixed, fromView_, view, t);
        view = mixed;
    }
    // The toast slides up when a message arrives and back down before it goes.
    const double shown = std::min({1.0, (st_.toastLength - st_.toastTime) / 0.25, st_.toastTime / 0.25});
    ui::toastBar(view, 0, 470, 800, st_.toast, shown);
    if (memes_.active()) memes_.render(view);
    c.blit(panel, 0, 0);
    c.blit(view, PanelW, 0);

    Polygon outline;
    if (drawHoverInCanvas && hoverOutline(outline)) {
        drawPolygonOutline(c, outline, SuspectColour::Highlight);
        drawPolygonOutline(c, apply(translate(1, 1), outline), SuspectColour::Highlight);
    }
    if (title_) renderTitle(c);
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
                          "SAFE: DRAG THE DIAL, ENTER LOCKS A NUMBER.",
                          "TERMINALS: TYPE CODE, F1 OR RUN, TAB SWITCHES FILE.",
                          "PRINTS: DRAG / SCROLL / ARROWS TO TURN, PICK A SUSPECT, COMPARE."};
    for (int i = 0; i < 5; ++i) drawText(c, W / 2 - textWidth(help[i]) / 2, 340 + i * 16, help[i], Color{170, 172, 186});
}

}  // namespace game
