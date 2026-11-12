#pragma once
// The playable prototype — Caleb (CP3, issue #47)
//
// Game ties every screen together into one investigation loop:
//   Explore -> drawer -> torn letter -> painting (code) -> laptop (code puzzle)
//   -> safe (combination) -> evidence board -> deduction (final build)
// It owns the GameState (Sudheysi), the screens (Alvin, Sudheysi, Latiffa) and
// the Case File panel (Latiffa), routes input to whichever screen is active,
// reacts to puzzles being solved, and cross-fades between screens.
//
// The window is 1050 x 500: Case File panel on the left (250 px), the active
// screen on the right (800 x 500). Game never touches GLFW or OpenGL, so the
// same code runs in the window (apps/the_last_clue.cpp) and in scripted tests
// (demos/32_prototype_playthrough.cpp).
#include "core/keys.h"
#include "explore.h"
#include "game_state.h"
#include "puzzles.h"
#include "ui/case_file_ui.h"
#include "ui/code_editor.h"

namespace game {

class Game {
public:
    static const int W = 1050, H = 500, PanelW = 250;
    Game();

    void update(double dt);
    void render(Canvas& c);                                  // full 1050 x 500 frame

    void mouseMove(PointF p);
    void mouseDown(PointF p, input::Button b);
    void mouseUp(PointF p, input::Button b);
    void scroll(PointF p, double dir);
    void keyDown(input::Key k);
    void character(unsigned codepoint);

    // Polygon to outline under the mouse (canvas coordinates), drawn by OpenGL.
    bool hoverOutline(Polygon& out) const;
    bool drawHoverInCanvas = false;                          // for screenshots without OpenGL

    const GameState& state() const { return st_; }
    bool onTitle() const { return title_; }

    // Access for scripted playthroughs / tests
    LetterPuzzle& letter() { return letter_; }
    SafePuzzle& safe() { return safe_; }
    ui::LaptopScreen& laptop() { return laptop_; }
    ui::BoardUI& board() { return board_; }
    ExploreScreen& explore() { return explore_; }

private:
    GameState st_;
    ExploreScreen explore_;
    PaintingView painting_;
    LetterPuzzle letter_;
    SafePuzzle safe_;
    ui::LaptopScreen laptop_;
    ui::BoardUI board_;
    ui::CaseFileUI panel_;

    bool title_ = true;
    PointF mouse_{0, 0};
    Screen lastScreen_ = Screen::Explore;
    Canvas lastView_{800, 500}, fromView_{800, 500};
    double fade_ = 1;                                        // 0 -> 1 cross-fade progress
    double letterTimer_ = -1;
    double time_ = 0;

    static PointF local(PointF p) { return {p.x - PanelW, p.y}; }
    void renderView(Canvas& view);
    void renderDeduction(Canvas& view) const;
    void renderTitle(Canvas& c) const;
    void onEnter(Screen s);
};

}  // namespace game
