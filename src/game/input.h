#pragma once
// Mouse interaction helpers — Sudheysi (CP3, issue #41)
//
// Picking:   which object is under the mouse? Each clickable object is a
//            polygon (a "hotspot"); pointInPolygon (ray casting, CP1) decides.
//            Hotspots later in the list are drawn on top, so we test from the
//            end of the list to the start and the first hit wins.
// Dragging:  remember where inside the object it was grabbed (grab offset) so
//            it doesn't jump to the cursor; clamp the result to the play area
//            (boundary constraint).
// Rotating:  for a dial, the angle of the mouse around the centre is atan2;
//            the change in that angle turns the dial.
#include <string>
#include <vector>

#include "core/canvas.h"
#include "polygon/polygon.h"

namespace game {

struct Hotspot {
    std::string id;
    Polygon area;
    std::string label;      // shown when hovered
};

// Topmost hotspot containing p, or nullptr.
const Hotspot* pick(const std::vector<Hotspot>& spots, PointF p);

struct Drag {
    bool active = false;
    int target = -1;
    PointF grabOffset{0, 0};    // object position - mouse position at grab time
};

PointF clampTo(PointF p, const ClipRect& area);
double angleAt(PointF centre, PointF p);       // radians, atan2
double angleBetween(double from, double to);   // shortest signed difference in (-pi, pi]
bool   inCircle(PointF centre, double r, PointF p);

}  // namespace game
