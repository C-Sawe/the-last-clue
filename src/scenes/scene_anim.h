#pragma once
// Crime scene + character animation — Alvin (CP2, issues #33 and #34)
#include "../core/canvas.h"

// How far each openable object is open: 0 = closed (exactly the CP1 scene), 1 = fully open.
struct SceneState {
    double door = 0;            // door swings into the room about its hinge
    double deskDrawer = 0;      // desk drawer slides out, showing the torn letter pieces
    double cabinetDrawer = 0;   // top cabinet drawer slides out
};

void renderCrimeSceneState(Canvas& c, const SceneState& s);   // 800 x 500

// One frame of the detective walking across the room, t in [0, 1].
void renderWalkFrame(Canvas& c, double t);                     // 800 x 500

// CP3 (issue #44): draw the detective (scaled sprite) with his feet at `feet`.
// walkPhase (radians) drives the leg/arm swing when walking.
void drawDetective(Canvas& c, PointF feet, double walkPhase, bool walking);
