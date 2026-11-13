#pragma once
// Final deduction screen and the outcome sequences — Latiffa (final build, issues #48 #52)
//
// DeductionUI: three columns (WHO? HOW? WHY?), each with three answer cards
// from Sudheysi's rules (game/deduction). The cards drop in one after another
// (staggered, OutBack easing - like the evidence board in CP2). Clicking a card
// selects it: the card is lifted (eased translation) and outlined; each card
// says whether evidence backs it. ACCUSE is enabled once all three are chosen.
//
// OutcomeUI: what happens after ACCUSE.
//   CASE SOLVED      - green; a rubber stamp SLAMS onto the screen (scaled down
//                      from 3x with an overshoot and rotated, drawn by inverse
//                      mapping), the story types itself out, and the partner's
//                      mug shot slides in marked ARRESTED.
//   WRONG ACCUSATION - amber stamp, what was wrong, REOPEN THE CASE.
//   CASE FAILED      - red stamp (no tries left); the partner walks away into
//                      the distance (scaled smaller about a vanishing point).
#include "core/keys.h"
#include "game/deduction.h"
#include "text.h"

namespace ui {

class DeductionUI {
public:
    enum class Action { None, Accuse };
    DeductionUI();
    void enter() { t_ = 0; }                       // replay the entrance animation
    void clear() { acc_ = game::Accusation{}; }
    void update(double dt);
    void mouseMove(PointF p) { mouse_ = p; }
    Action mouseDown(PointF p);
    void render(Canvas& c, const game::GameState& st) const;      // 800 x 500

    const game::Accusation& accusation() const { return acc_; }
    void pick(int question, int answer) { acc_.pick[question] = answer; }
    PointF cardCentre(int question, int answer) const;           // for scripted demos
    PointF accuseCentre() const { return {accuse_.x + accuse_.w / 2, accuse_.y + accuse_.h / 2}; }

private:
    game::Accusation acc_;
    double t_ = 0;
    double lift_[game::QuestionCount][game::AnswerCount] = {};    // 0 -> 1 as a card is selected
    PointF mouse_{0, 0};
    Button accuse_;
    int cardAt(PointF p, int& question) const;
};

class OutcomeUI {
public:
    enum class Action { None, PlayAgain, Reopen };
    void show(const game::Verdict& v);
    void update(double dt) { t_ += dt; }
    void mouseMove(PointF p) { mouse_ = p; }
    Action mouseDown(PointF p) const;
    Action key(input::Key k) const;
    void render(Canvas& c) const;                  // 800 x 500
    bool ready() const { return t_ >= ButtonAt; }  // the button has appeared
    PointF buttonCentre() const { return {button().x + button().w / 2, button().y + button().h / 2}; }
    const game::Verdict& verdict() const { return v_; }

    static constexpr double ButtonAt = 2.6;        // seconds into the sequence

private:
    game::Verdict v_;
    double t_ = 0;
    PointF mouse_{0, 0};
    Button button() const;
};

}  // namespace ui
