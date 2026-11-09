#pragma once
// Interactive Case File panel and evidence board — Latiffa (CP3, issue #45)
//
// CaseFileUI: the left panel of the game window. Unlike the CP1 picture it is
// LIVE: evidence slots fill in as the GameState collects evidence, and its
// buttons (board, deduction, back) respond to hover and clicks.
//
// BoardUI: the interactive evidence board. Every card can be DRAGGED (grab
// offset + clamped to the cork). Clicking one pin and then another ties a
// string between them (a sagging quadratic Bézier); clicking the same pair
// again removes it. The strings are stored in GameState::links so they survive
// leaving and re-entering the board.
#include <vector>

#include "game/game_state.h"
#include "game/input.h"
#include "text.h"

namespace ui {

class CaseFileUI {
public:
    enum class Action { None, Board, Deduction, Back };
    CaseFileUI();
    void mouseMove(PointF p) { mouse_ = p; }
    Action click(PointF p, const game::GameState& st) const;
    void render(Canvas& c, const game::GameState& st) const;      // 250 x 500

private:
    PointF mouse_{-1, -1};
    Button board_, deduce_, back_;
};

class BoardUI {
public:
    BoardUI();
    void sync(const game::GameState& st);          // add cards for newly found evidence
    void mouseDown(PointF p, game::GameState& st);
    void mouseMove(PointF p);
    void mouseUp(PointF p);
    void render(Canvas& c, const game::GameState& st) const;      // 800 x 500

    // ids used in GameState::links: 0 victim, 1-3 suspects, 10 + Evidence for evidence cards
    PointF pinOf(int id) const;                    // for scripted demos
    PointF cardCentre(int id) const;

private:
    struct Card { int id; double x, y, w, h; };
    std::vector<Card> cards_;
    game::Drag drag_;
    int selectedPin_ = -1;
    PointF mouse_{0, 0};

    int cardAt(PointF p) const;
    int pinAt(PointF p) const;
};

}  // namespace ui
