#pragma once
// Puzzle animations — Sudheysi (CP2, issues #35, #36, #37)
#include <vector>

#include "../core/canvas.h"
#include "../polygon/polygon.h"
#include "../transform/transform2d.h"

// Safe close-up: the dial at `dialAngle` and the whole door plate swung open by
// doorOpen (0 = shut, exactly the CP1 dial; 1 = open, interior visible).   420 x 420
void renderSafeOpening(Canvas& c, double dialAngle, double doorOpen);

// Torn letter at progress 0 (scattered, the CP1 puzzle) -> 1 (solved, the CP1
// solution). In between, every piece interpolates its rotation and translation.  600 x 480
void renderTornDocumentAt(Canvas& c, double progress);

// The crime scene seen through the magnifying glass: a lens window centred on
// the screen point `focus`, showing the scene scaled by `zoom` about that point.  800 x 500
void renderMagnifierAt(Canvas& c, PointF focus, double zoom);

// ---- CP3 (issue #43): pieces of the torn letter for the interactive puzzle ----
// A pose places piece i on screen: rotate by `angle` about the piece's own
// centre, then move the letter's top-left corner to `origin + (dx, dy)`.
struct TornPose { double angle = 0, dx = 0, dy = 0; };

int     tornPieceCount();                                 // 4
Polygon tornPiece(int i);                                 // in letter coordinates (0..260 x 0..330)
PointF  tornPieceCentre(int i);                           // centroid, letter coordinates
PointF  tornLetterOrigin(int canvasW, int canvasH);       // where the solved letter sits
TornPose tornSolvedPose(int i);                           // the CP1/CP2 solved pose (small gaps)
Mat3    tornPoseMatrix(int i, const TornPose& pose, int canvasW, int canvasH);

// Draws the desk and all pieces in the given poses, in the order given by
// `drawOrder` (last = on top). `highlight` (or -1) gets a yellow outline,
// `locked[i]` pieces get a thin green outline.
void renderTornPieces(Canvas& c, const std::vector<TornPose>& poses, const std::vector<int>& drawOrder,
                      const std::vector<bool>& locked, int highlight);
