#pragma once
// Owner: Caleb  —  Issues: Lines (DDA + Bresenham), Midpoint circle, Midpoint ellipse
// Implement these in line.cpp / circle.cpp / ellipse.cpp in this folder.
#include "../core/canvas.h"

// ---- Lines ----
void drawLineDDA(Canvas& c, Point a, Point b, Color col);
void drawLineBresenham(Canvas& c, Point a, Point b, Color col);   // all 8 octants
void drawThickLine(Canvas& c, Point a, Point b, int thickness, Color col);

// ---- Circles ----
void drawCircleMidpoint(Canvas& c, Point centre, int radius, Color col);
void fillCircle(Canvas& c, Point centre, int radius, Color col);

// ---- Ellipses ----
void drawEllipseMidpoint(Canvas& c, Point centre, int rx, int ry, Color col);
void fillEllipse(Canvas& c, Point centre, int rx, int ry, Color col);
