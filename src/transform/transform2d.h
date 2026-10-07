#pragma once
// Owner: Caleb  —  Issue: 2D transformation module
// Implement in transform2d.cpp. Uses 3x3 homogeneous matrices so transforms compose.
#include "../core/canvas.h"
#include "../polygon/polygon.h"

struct Mat3 {
    double m[3][3];
    static Mat3 identity();
    Mat3 operator*(const Mat3& o) const;
};

Mat3 translate(double tx, double ty);              // x' = x + tx,         y' = y + ty
Mat3 scale(double sx, double sy);                  // x' = sx*x,           y' = sy*y
Mat3 rotate(double radians);                       // x' = x cosθ - y sinθ, y' = x sinθ + y cosθ
Mat3 rotateAbout(double radians, PointF pivot);    // T(p) * R * T(-p)
Mat3 scaleAbout(double sx, double sy, PointF pivot);

PointF  apply(const Mat3& m, PointF p);
Polygon apply(const Mat3& m, const Polygon& poly);
