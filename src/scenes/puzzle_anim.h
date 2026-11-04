#pragma once
// Puzzle animations — Sudheysi (CP2, issues #35, #36, #37)
#include "../core/canvas.h"

// Safe close-up: the dial at `dialAngle` and the whole door plate swung open by
// doorOpen (0 = shut, exactly the CP1 dial; 1 = open, interior visible).   420 x 420
void renderSafeOpening(Canvas& c, double dialAngle, double doorOpen);

// Torn letter at progress 0 (scattered, the CP1 puzzle) -> 1 (solved, the CP1
// solution). In between, every piece interpolates its rotation and translation.  600 x 480
void renderTornDocumentAt(Canvas& c, double progress);

// The crime scene seen through the magnifying glass: a lens window centred on
// the screen point `focus`, showing the scene scaled by `zoom` about that point.  800 x 500
void renderMagnifierAt(Canvas& c, PointF focus, double zoom);
