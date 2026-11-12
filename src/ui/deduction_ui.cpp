// Final deduction screen and the outcome sequences — Latiffa (final build, issues #48 #52)
#include "deduction_ui.h"

#include <algorithm>
#include <cmath>

#include "animation/animation.h"
#include "core/font.h"
#include "polygon/polygon.h"
#include "primitives/primitives.h"
#include "scenes/scenes.h"
#include "transform/transform2d.h"

namespace ui {

namespace {
const double ColX = 20, ColW = 240, ColGap = 260, CardY = 104, CardH = 86, CardGap = 96;
const Color Bg{24, 22, 30}, CardCol{48, 46, 60}, CardEdge{90, 88, 110};

// Draws `src` onto `dst` scaled by k and rotated by `angle` about `centre`,
// skipping pixels of colour `key`. Inverse mapping: every destination pixel in
// the bounding box is mapped BACK into the source, so there are no holes.
void drawTransformed(Canvas& dst, const Canvas& src, PointF centre, double angle, double k, Color key) {
    const double hw = src.width() / 2.0, hh = src.height() / 2.0;
    const Mat3 M = translate(centre.x, centre.y) * rotate(angle) * scale(k, k) * translate(-hw, -hh);
    const Mat3 back = inverse(M);
    const double r = std::hypot(hw, hh) * k + 2;
    for (int y = std::max(0, static_cast<int>(centre.y - r)); y <= std::min(dst.height() - 1, static_cast<int>(centre.y + r)); ++y)
        for (int x = std::max(0, static_cast<int>(centre.x - r)); x <= std::min(dst.width() - 1, static_cast<int>(centre.x + r)); ++x) {
            const PointF s = apply(back, PointF{x + 0.5, y + 0.5});
            if (s.x < 0 || s.y < 0 || s.x >= src.width() || s.y >= src.height()) continue;
            const Color col = src.getPixel(static_cast<int>(s.x), static_cast<int>(s.y));
            if (!(col == key)) dst.setPixel(x, y, col);
        }
}

// A rubber stamp: double border + big letters, in its own canvas.
Canvas makeStamp(const std::string& text, Color ink) {
    const int s = 5, w = textWidth(text, s) + 40, h = 7 * s + 34;
    const Color key{255, 0, 255};
    Canvas c(w, h, key);
    drawPolygonOutline(c, makeRect(2, 2, w - 5, h - 5), ink);
    drawPolygonOutline(c, makeRect(3, 3, w - 7, h - 7), ink);
    drawPolygonOutline(c, makeRect(8, 8, w - 17, h - 17), ink);
    drawText(c, 20, 17, text, ink, s);
    // worn ink: knock out a few pixels in a fixed pattern
    unsigned seed = 5;
    for (int i = 0; i < w * h / 14; ++i) {
        seed = seed * 1103515245u + 12345u;
        const int x = static_cast<int>((seed >> 8) % static_cast<unsigned>(w));
        seed = seed * 1103515245u + 12345u;
        const int y = static_cast<int>((seed >> 8) % static_cast<unsigned>(h));
        if (c.getPixel(x, y) == ink) c.setPixel(x, y, key);
    }
    return c;
}

// The partner, head and shoulders (same construction as the board's photos).
void partnerFigure(Canvas& c, double cx, double feetY, double k) {
    const Color skin{205, 160, 120};
    const int x = static_cast<int>(cx);
    fillEllipse(c, {x, static_cast<int>(feetY - 70 * k)}, static_cast<int>(26 * k), static_cast<int>(60 * k), SuspectColour::Partner);   // coat
    fillEllipse(c, {x, static_cast<int>(feetY - 140 * k)}, static_cast<int>(14 * k), static_cast<int>(17 * k), skin);                // head
    scanlineFill(c, makeRect(cx - 18 * k, feetY - 160 * k, 36 * k, 6 * k), Color{30, 30, 34});                                       // hat brim
    scanlineFill(c, makeRect(cx - 11 * k, feetY - 176 * k, 22 * k, 16 * k), Color{30, 30, 34});                                      // hat
}

void mugShot(Canvas& c, double x, double y) {
    scanlineFill(c, makeRect(x + 5, y + 6, 170, 210), Color{10, 10, 12});
    scanlineFill(c, makeRect(x, y, 170, 210), Color{240, 238, 230});
    scanlineFill(c, makeRect(x + 10, y + 10, 150, 150), Color{170, 176, 188});
    for (int i = 0; i < 5; ++i)                                                     // height chart
        drawLineBresenham(c, {static_cast<int>(x) + 10, static_cast<int>(y) + 30 + i * 28},
                          {static_cast<int>(x) + 40, static_cast<int>(y) + 30 + i * 28}, Color{120, 124, 136});
    Canvas photo(150, 150, Color{170, 176, 188});
    partnerFigure(photo, 75, 300, 1.4);
    c.blit(photo, static_cast<int>(x) + 10, static_cast<int>(y) + 10);
    drawText(c, static_cast<int>(x) + 85 - textWidth("THE PARTNER", 2) / 2, static_cast<int>(y) + 168, "THE PARTNER", Color{40, 40, 40}, 2);
    drawText(c, static_cast<int>(x) + 85 - textWidth("CASE 47") / 2, static_cast<int>(y) + 190, "CASE 47", Color{110, 110, 120});
}
}  // namespace

// =============================== DeductionUI ===============================
DeductionUI::DeductionUI() { accuse_ = {300, 410, 200, 40, "ACCUSE"}; }

PointF DeductionUI::cardCentre(int q, int a) const { return {ColX + q * ColGap + ColW / 2, CardY + a * CardGap + CardH / 2}; }

int DeductionUI::cardAt(PointF p, int& question) const {
    for (int q = 0; q < game::QuestionCount; ++q)
        for (int a = 0; a < game::AnswerCount; ++a) {
            const PointF c = cardCentre(q, a);
            if (std::fabs(p.x - c.x) <= ColW / 2 && std::fabs(p.y - c.y) <= CardH / 2) { question = q; return a; }
        }
    return -1;
}

void DeductionUI::update(double dt) {
    t_ += dt;
    // Each card's lift moves smoothly towards 1 (selected) or 0: exponential easing.
    const double k = 1 - std::exp(-dt * 14);
    for (int q = 0; q < game::QuestionCount; ++q)
        for (int a = 0; a < game::AnswerCount; ++a) lift_[q][a] += ((acc_.pick[q] == a ? 1.0 : 0.0) - lift_[q][a]) * k;
}

DeductionUI::Action DeductionUI::mouseDown(PointF p) {
    if (accuse_.contains(p)) return acc_.complete() ? Action::Accuse : Action::None;
    int q = 0;
    const int a = cardAt(p, q);
    if (a >= 0) acc_.pick[q] = (acc_.pick[q] == a) ? -1 : a;     // click again to un-pick
    return Action::None;
}

void DeductionUI::render(Canvas& c, const game::GameState& st) const {
    c.clear(Bg);
    drawText(c, 400 - textWidth("FINAL DEDUCTION", 3) / 2, 16, "FINAL DEDUCTION", Palette::White, 3);
    drawText(c, 400 - textWidth("PICK ONE ANSWER IN EACH COLUMN, THEN ACCUSE") / 2, 46, "PICK ONE ANSWER IN EACH COLUMN, THEN ACCUSE",
             Color{170, 172, 186});

    for (int q = 0; q < game::QuestionCount; ++q) {
        const auto question = static_cast<game::Question>(q);
        const int hx = static_cast<int>(ColX + q * ColGap + ColW / 2);
        drawText(c, hx - textWidth(game::questionName(question), 3) / 2, 70, game::questionName(question), SuspectColour::Highlight, 3);
        for (int a = 0; a < game::AnswerCount; ++a) {
            // Entrance: cards drop in one after another with an overshoot.
            const int order = q * game::AnswerCount + a;
            const double land = anim::phase(t_, 0.05 * order, 0.05 * order + 0.45);
            if (land <= 0) continue;
            const double dy = anim::lerp(-520, 0, anim::ease(anim::Ease::OutBack, land)) - 6 * lift_[q][a];
            const double x = ColX + q * ColGap, y = CardY + a * CardGap + dy;
            const bool chosen = acc_.pick[q] == a;
            const bool hover = std::fabs(mouse_.x - (x + ColW / 2)) <= ColW / 2 && std::fabs(mouse_.y - (y + CardH / 2)) <= CardH / 2;
            const game::Answer& ans = game::answer(question, a);
            const bool backed = game::answerSupported(st, question, a);

            scanlineFill(c, makeRect(x + 4, CardY + a * CardGap + 5, ColW, CardH), Color{12, 12, 16});   // shadow stays put
            scanlineFill(c, makeRect(x, y, ColW, CardH), chosen ? Color{62, 60, 80} : (hover ? Color{56, 54, 70} : CardCol));
            const Color edge = chosen ? SuspectColour::Highlight : CardEdge;
            drawPolygonOutline(c, makeRect(x, y, ColW, CardH), edge);
            if (chosen) drawPolygonOutline(c, makeRect(x + 1, y + 1, ColW - 2, CardH - 2), edge);
            if (q == 0) fillCircle(c, {static_cast<int>(x) + 18, static_cast<int>(y) + 18}, 8,
                                   a == 0 ? SuspectColour::Butler : (a == 1 ? SuspectColour::Heiress : SuspectColour::Partner));
            drawText(c, static_cast<int>(x) + (q == 0 ? 32 : 10), static_cast<int>(y) + 11, ans.title, Palette::White, 2);
            drawText(c, static_cast<int>(x) + 10, static_cast<int>(y) + 36, ans.detail, Color{170, 172, 186});
            if (backed) {
                // a tick: two Bresenham lines
                const int tx = static_cast<int>(x) + 10, ty = static_cast<int>(y) + 62;
                drawThickLine(c, {tx, ty + 3}, {tx + 3, ty + 6}, 2, Feedback::Correct);
                drawThickLine(c, {tx + 3, ty + 6}, {tx + 9, ty - 1}, 2, Feedback::Correct);
                drawText(c, tx + 16, ty, ans.support, Feedback::Correct);
            } else {
                drawText(c, static_cast<int>(x) + 10, static_cast<int>(y) + 62, "NO EVIDENCE FOR THIS", Color{110, 112, 126});
            }
        }
    }

    drawButton(c, accuse_, acc_.complete(), accuse_.contains(mouse_), Color{170, 50, 50});
    const int left = game::MaxAccusations - st.accusations;
    const std::string tries = std::to_string(left) + (left == 1 ? " ACCUSATION LEFT" : " ACCUSATIONS LEFT");
    drawText(c, 400 - textWidth(tries) / 2, 456, tries, left == 1 ? Feedback::Wrong : Color{170, 172, 186});
}

// ================================ OutcomeUI ================================
void OutcomeUI::show(const game::Verdict& v) {
    v_ = v;
    t_ = 0;
}

Button OutcomeUI::button() const {
    const bool again = v_.solved || v_.caseClosed;
    return {290, 440, 220, 40, again ? "PLAY AGAIN" : "REOPEN THE CASE"};
}

OutcomeUI::Action OutcomeUI::mouseDown(PointF p) const {
    if (!ready() || !button().contains(p)) return Action::None;
    return (v_.solved || v_.caseClosed) ? Action::PlayAgain : Action::Reopen;
}

OutcomeUI::Action OutcomeUI::key(input::Key k) const {
    if (!ready() || (k != input::Key::Enter && k != input::Key::Space)) return Action::None;
    return (v_.solved || v_.caseClosed) ? Action::PlayAgain : Action::Reopen;
}

void OutcomeUI::render(Canvas& c) const {
    using anim::Ease;
    const Color tint = v_.solved ? Color{16, 40, 26} : (v_.caseClosed ? Color{48, 12, 14} : Color{46, 34, 12});
    const Color ink = v_.solved ? Color{90, 210, 120} : (v_.caseClosed ? Color{230, 70, 70} : Color{240, 170, 60});

    // 1. Background fades in from black.
    c.clear(anim::lerp(Color{0, 0, 0}, tint, anim::ease(Ease::OutQuad, anim::phase(t_, 0, 0.5))));
    for (int y = 0; y < c.height(); y += 4)                                   // faint paper lines
        drawLineBresenham(c, {0, y}, {c.width() - 1, y}, anim::lerp(tint, Color{0, 0, 0}, 0.25));

    // 2. Side picture: the arrest (mug shot slides in) or the partner walking away.
    if (v_.solved) {
        const double s = anim::ease(Ease::OutQuad, anim::phase(t_, 1.2, 1.9));
        if (s > 0) {
            const double x = anim::lerp(820, 600, s);
            mugShot(c, x, 170);
            if (t_ > 1.9) drawText(c, static_cast<int>(x) + 85 - textWidth("ARRESTED", 3) / 2, 136, "ARRESTED", Feedback::Wrong, 3);
        }
    } else if (v_.caseClosed) {
        // Walking away towards a vanishing point: scale shrinks about the horizon point.
        const double w = anim::ease(Ease::InOutCubic, anim::phase(t_, 1.0, 3.2));
        const PointF vanish{690, 260};
        const double k = anim::lerp(1.0, 0.18, w);
        drawLineBresenham(c, {560, 470}, {static_cast<int>(vanish.x), static_cast<int>(vanish.y)}, ink);      // road edges
        drawLineBresenham(c, {800, 470}, {static_cast<int>(vanish.x), static_cast<int>(vanish.y)}, ink);
        partnerFigure(c, anim::lerp(680, vanish.x, w), anim::lerp(460, vanish.y, w), k);
        if (t_ > 2.6) drawText(c, 600, 120, "HE WALKS FREE", ink, 2);
    }

    // 3. The stamp slams down: scale 3x -> 1x with an overshoot, slightly rotated,
    //    then a short shake on impact.
    const double slam = anim::phase(t_, 0.3, 0.9);
    if (slam > 0) {
        static Canvas stamps[3] = {makeStamp("CASE SOLVED", Color{90, 210, 120}), makeStamp("CASE FAILED", Color{230, 70, 70}),
                                   makeStamp("WRONG ACCUSATION", Color{240, 170, 60})};
        const Canvas& st = stamps[v_.solved ? 0 : (v_.caseClosed ? 1 : 2)];
        const double k = anim::lerp(3.0, 1.0, anim::ease(Ease::OutBack, slam));
        const double shake = (t_ > 0.9 && t_ < 1.15) ? std::sin(t_ * 90) * 4 * (1.15 - t_) / 0.25 : 0;
        const bool sidePicture = v_.solved || v_.caseClosed;          // leave room on the right for it
        drawTransformed(c, st, {(sidePicture ? 290.0 : 400.0) + shake, 90}, -0.12, k * (sidePicture ? 1.0 : 0.9), Color{255, 0, 255});
    }

    // 4. The story types itself out, 45 characters a second.
    int budget = static_cast<int>(std::max(0.0, t_ - 1.0) * 45);
    const int textW = (v_.solved || v_.caseClosed) ? 52 : 70;
    int y = 190;
    for (const auto& line : v_.lines)
        for (const auto& part : wrap(line, textW)) {
            if (budget <= 0) break;
            const std::string shown = part.substr(0, static_cast<size_t>(std::min<int>(budget, static_cast<int>(part.size()))));
            budget -= static_cast<int>(part.size());
            drawText(c, 40, y, shown, Palette::White);
            y += 18;
        }
    if (!v_.solved) {
        const std::string left = v_.caseClosed ? "NO ACCUSATIONS LEFT - THE CASE IS CLOSED"
                                               : std::to_string(v_.correct) + " OF 3 RIGHT.  " + std::to_string(v_.triesLeft) +
                                                     (v_.triesLeft == 1 ? " ACCUSATION LEFT" : " ACCUSATIONS LEFT");
        if (t_ > 1.6) drawText(c, 40, 380, left, ink, 2);
    }

    // 5. The button fades in last.
    if (t_ >= ButtonAt) {
        const Button b = button();
        drawButton(c, b, true, b.contains(mouse_), anim::lerp(tint, ink, std::min(1.0, (t_ - ButtonAt) / 0.4) * 0.6));
    }
}

}  // namespace ui
