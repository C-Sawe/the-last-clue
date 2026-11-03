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

// CP2 additions
Mat3 shear(double shx, double shy);                // x' = x + shx*y,      y' = y + shy*x
Mat3 shearAbout(double shx, double shy, PointF pivot);
Mat3 reflectX();                                   // mirror in the x-axis:  y' = -y
Mat3 reflectY();                                   // mirror in the y-axis:  x' = -x
Mat3 reflectAbout(PointF p, double lineAngle);     // mirror in any line through p at angle

Mat3 inverse(const Mat3& m);                       // undo a transform (used to map pixels back)

PointF  apply(const Mat3& m, PointF p);
Polygon apply(const Mat3& m, const Polygon& poly);
