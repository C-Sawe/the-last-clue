#pragma once
// Owner: Latiffa  —  Issues: Bézier (de Casteljau), Catmull-Rom spline
// Implement in bezier.cpp / spline.cpp in this folder.
#include <vector>
#include "../core/canvas.h"

// Evaluate a Bézier curve of any degree at t in [0,1] using de Casteljau.
PointF bezierPoint(const std::vector<PointF>& control, double t);
void   drawBezier(Canvas& c, const std::vector<PointF>& control, Color col, int segments = 64);

// Catmull-Rom spline passing through every point in `pts`.
void drawCatmullRom(Canvas& c, const std::vector<PointF>& pts, Color col, int segmentsPerSpan = 24);

// Debug helper: draw control points + control polygon (useful in screenshots).
void drawControlPolygon(Canvas& c, const std::vector<PointF>& control, Color col);
