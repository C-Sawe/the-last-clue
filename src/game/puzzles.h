#pragma once
// Interactive puzzles — Sudheysi (CP3, issue #43)
// Both puzzles draw into an 800 x 500 canvas (the Crime Scene viewport) and take
// mouse positions in that canvas's coordinates.
#include <string>
#include <vector>

#include "core/keys.h"
#include "input.h"
#include "scenes/puzzle_anim.h"

namespace game {

// ---------------------------------------------------------------------------
// Torn letter: drag pieces (translate), rotate with the mouse wheel or R
// (15 degrees per step), and they SNAP into place when close enough to the
// solved pose (distance < 16 px and angle within ~11 degrees). Snapped pieces lock.
// ---------------------------------------------------------------------------
class LetterPuzzle {
public:
    LetterPuzzle();
    void reset();

    void mouseDown(PointF p);
    void mouseMove(PointF p);
    void mouseUp(PointF p);
    void scroll(PointF p, double dir);
    void key(input::Key k, PointF mouse);

    void render(Canvas& c) const;                 // 800 x 500
    bool solved() const;
    int  placed() const;

    // Helpers for scripted demos / tests
    PointF pieceScreenCentre(int i) const;
    PointF solvedScreenCentre(int i) const;
    double pieceAngle(int i) const { return poses_[static_cast<size_t>(i)].angle; }

private:
    std::vector<TornPose> poses_;
    std::vector<bool> locked_;
    std::vector<int> order_;                      // draw order, last = on top
    Drag drag_;
    int hover_ = -1;

    int pieceAt(PointF p) const;
    void trySnap(int i);
    void bringToFront(int i);
};

// ---------------------------------------------------------------------------
// Safe dial: drag around the dial to turn it (change in mouse angle = change
// in dial angle). ENTER (or the SET button) locks in the number under the
// pointer. Three correct numbers -> the door swings open.
// ---------------------------------------------------------------------------
class SafePuzzle {
public:
    explicit SafePuzzle(std::vector<int> combination = {37, 12, 85});
    void reset();

    void mouseDown(PointF p);
    void mouseMove(PointF p);
    void mouseUp(PointF p);
    void key(input::Key k);
    void update(double dt);

    void render(Canvas& c) const;                 // 800 x 500
    int  number() const;                          // number under the pointer
    bool solved() const { return solved_; }
    bool doorOpen() const { return doorT_ >= 1; }
    std::string takeMessage();                    // toast for the game, then cleared

    PointF dialCentre() const { return {400, 260}; }   // screen position (for scripted demos)

private:
    std::vector<int> combo_, entered_;
    double angle_ = 0, grabAngle_ = 0, grabDial_ = 0;
    bool dragging_ = false, solved_ = false;
    double doorT_ = 0, shake_ = 0;
    std::string message_;

    void enterNumber();
};

// ---------------------------------------------------------------------------
// Fingerprint matching (final build, issue #50): the print lifted from the safe
// was found at an angle. Turn it (drag around it, scroll, R, or the arrow keys) and pick
// the suspect whose print it matches. The match score is the INTERSECTION OVER
// UNION of the two prints' ink: the lifted print is rotated by inverse mapping,
// both prints become masks (thickened by one pixel so rounding doesn't matter),
// and IoU = |A and B| / |A or B|. COMPARE passes at IoU >= MatchThreshold.
// Turning snaps to 5-degree steps; the right suspect scores 100% when lined up
// and ~55% at 5 degrees off, while the wrong suspects stay below 50% at any angle.
// ---------------------------------------------------------------------------
class PrintsPuzzle {
public:
    static constexpr double MatchThreshold = 0.53;
    static constexpr double StartAngle = 1.0471975511965976;     // 60 degrees off

    PrintsPuzzle();
    void reset();
    void mouseDown(PointF p);
    void mouseMove(PointF p);
    void mouseUp(PointF p);
    void scroll(PointF p, double dir);
    void key(input::Key k);
    void render(Canvas& c) const;                 // 800 x 500

    void select(int suspect) { selected_ = suspect; }
    void setAngle(double a) { angle_ = a; }
    double angle() const { return angle_; }
    int selected() const { return selected_; }
    double score() const;                         // IoU with the selected suspect (0 if none)
    void compare();                               // COMPARE button
    bool solved() const { return solved_; }
    std::string takeMessage();

    PointF printCentre() const { return {150, 230}; }    // for scripted demos
    PointF cardCentre(int suspect) const;

private:
    double angle_ = StartAngle, grabAngle_ = 0, grabStart_ = 0;
    bool dragging_ = false, solved_ = false;
    int selected_ = -1, hover_ = -1;
    PointF mouse_{0, 0};
    std::string message_;
    int cardAt(PointF p) const;
};

// Exposed for demos and tests: IoU between the lifted print (turned by `angle`)
// and a suspect's print, both at `size` pixels.
double printMatchScore(int suspect, double angle, int size = 160);

}  // namespace game
