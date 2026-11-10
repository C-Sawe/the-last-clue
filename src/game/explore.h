#pragma once
// The playable crime scene — Alvin (CP3, issue #44)
//
// ExploreScreen: the study from CP1/CP2, now interactive.
//   - Every clickable object is a hotspot polygon (picked with point-in-polygon).
//   - Hovering shows the object's name; the game draws its outline with OpenGL.
//   - Clicking sends the detective WALKING to the object (translation over time,
//     walk-cycle limbs from CP2); when he arrives the object's action runs:
//     the drawer slides open, the door is tried, the safe/laptop/painting open, ...
//   - A laptop sits on the filing cabinet (the code puzzle), its screen red
//     (alarm armed) or green (disabled).
//
// PaintingView: close-up of the painting. Once the letter is solved, the
// painting swings aside about its nail (rotation about a pivot, drawn by inverse
// mapping) and reveals the safe combination and the victim's star symbol.
#include <string>
#include <vector>

#include "game_state.h"
#include "input.h"

namespace game {

class ExploreScreen {
public:
    ExploreScreen();

    void mouseMove(PointF p);
    void click(PointF p, GameState& st);
    void update(double dt, GameState& st);
    void render(Canvas& c, const GameState& st) const;           // 800 x 500

    const std::vector<Hotspot>& hotspots() const { return spots_; }
    const Hotspot* hovered() const { return pick(spots_, mouse_); }
    bool busy() const { return !pending_.empty(); }              // detective still walking
    PointF detectivePos() const { return {x_, 462}; }

private:
    std::vector<Hotspot> spots_;
    PointF mouse_{-1, -1};
    double x_ = 700, targetX_ = 700, phase_ = 0;
    std::string pending_;                                        // action to run on arrival
    double drawerT_ = 0, doorT_ = 0;
    bool openingDrawer_ = false;

    void arrive(GameState& st);
    double standX(const std::string& id) const;
};

class PaintingView {
public:
    void enter() { t_ = 0; }
    void update(double dt, GameState& st);
    void render(Canvas& c) const;                                // 800 x 500
    bool revealed() const { return t_ >= 1; }

private:
    double t_ = 0;
};

}  // namespace game
