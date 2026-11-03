#pragma once
// Scene transitions — Latiffa (CP2, issue #32)
// Each function writes into `out` a blend of scene `from` and scene `to` at
// progress t in [0,1]. All three canvases must be the same size.
#include "../core/canvas.h"

namespace anim {

// Cross-fade: every pixel is lerp(from, to, t)  — colour interpolation.
void crossFade(Canvas& out, const Canvas& from, const Canvas& to, double t);

// Fade through black: first half darkens `from`, second half brightens `to`.
void fadeThroughBlack(Canvas& out, const Canvas& from, const Canvas& to, double t);

// Wipe: `to` is revealed inside a clip window that grows from the left edge.
// The window is a rectangle clipped with Sutherland–Hodgman; a bright edge line
// (Cohen–Sutherland clipped) marks the moving boundary.
void wipe(Canvas& out, const Canvas& from, const Canvas& to, double t);

// Iris: `to` appears inside a growing circle (point-in-circle test per pixel)
// while `from` dims outside it - the classic detective-film transition.
void iris(Canvas& out, const Canvas& from, const Canvas& to, double t, PointF centre);

}  // namespace anim
