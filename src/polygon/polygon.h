#pragma once
// Owners:
//   Alvin    — Polygon representation, scanline fill, flood/boundary fill  (polygon.cpp, fill.cpp)
//   Sudheysi — Cohen–Sutherland, Sutherland–Hodgman, point-in-polygon      (clip.cpp)
#include <vector>
#include "../core/canvas.h"

// ---- Representation (Alvin) ----
struct Polygon {
    std::vector<PointF> vertices;   // in order; last vertex connects back to first
};

Polygon makeRect(double x, double y, double w, double h);
void    drawPolygonOutline(Canvas& c, const Polygon& poly, Color col);  // uses Bresenham

// ---- Filling (Alvin) ----
void scanlineFill(Canvas& c, const Polygon& poly, Color fill);
void floodFill(Canvas& c, Point seed, Color fill);                          // 4-connected
void boundaryFill(Canvas& c, Point seed, Color fill, Color boundary);

// ---- Clipping (Sudheysi) ----
struct ClipRect { double xmin, ymin, xmax, ymax; };

// Returns false if the line lies completely outside; otherwise a and b are clipped in place.
bool    cohenSutherlandClip(PointF& a, PointF& b, const ClipRect& r);
Polygon sutherlandHodgmanClip(const Polygon& subject, const ClipRect& r);

// ---- Hit testing (Sudheysi) — reused later for mouse picking ----
bool pointInPolygon(const Polygon& poly, PointF p);
