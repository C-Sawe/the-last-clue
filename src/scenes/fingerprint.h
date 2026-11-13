#pragma once
// Fingerprint art — Alvin (final build, issue #49)
//
// Every print is built from CONCENTRIC MIDPOINT ELLIPSES (CP1), shaped into the
// three classic ridge patterns:
//   Whorl - full concentric ellipses around the core
//   Loop  - the top halves of the ellipses, with both ends running down and
//           sliding sideways (a "U" lying on its side)
//   Arch  - very wide, flat ellipses whose centre sits below the finger, so only
//           their tops cross the print
// A scar is a thick paper-coloured line cutting the ridges. The finger outline
// is an ellipse; ridges outside it are erased (point-in-ellipse test).
//
// Prints are drawn axis-aligned into their own canvas, then placed on screen
// ROTATED with inverse mapping (the same raster rotation as the painting), so
// the matching puzzle can turn the lifted print to line it up.
#include "../core/canvas.h"

struct PrintSpec {
    enum class Pattern { Whorl, Loop, Arch };
    Pattern pattern = Pattern::Whorl;
    int ridges = 9;             // number of ridge lines
    double gap = 6;             // pixels between ridges (in a 160 px print)
    double stretch = 1.25;      // ry / rx of each ridge ellipse
    PointF core{0, 0};          // centre of the pattern, offset from the middle
    double slant = 0;           // Loop: how far the ends slide sideways (px)
    bool scar = false;          // a scar across the ridges
    double scarAngle = 0;       // radians
};

// Suspect index: 0 = the butler, 1 = the heiress, 2 = the partner.
const PrintSpec& suspectPrint(int suspect);
const PrintSpec& foundPrint();              // the print lifted from the safe
const char* printPatternName(const PrintSpec& s);

// The print drawn axis-aligned into a size x size canvas (ink on paper).
Canvas renderPrint(const PrintSpec& s, int size, Color ink, Color paper);

// Draws `print` (from renderPrint) centred at `centre`, rotated by `angle`
// radians, using inverse mapping. Pixels equal to `paper` are skipped when
// `transparent`, so two prints can be overlaid.
void drawPrintRotated(Canvas& c, const Canvas& print, PointF centre, double angle, Color paper, bool transparent = true);

// A suspect's fingerprint card for the matching line-up (name strip in the
// suspect's colour). `selected` raises the card and outlines it.
void drawPrintCard(Canvas& c, double x, double y, int suspect, bool selected, bool hovered);
const int PrintCardW = 150, PrintCardH = 190;
