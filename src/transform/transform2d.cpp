// 2D transformations with 3x3 homogeneous matrices — Caleb
//
// A point (x, y) is written as the column vector [x, y, 1]^T.
// Using a 3x3 matrix lets translation be a matrix multiply too,
// so any chain of transforms collapses into ONE matrix:
//   M = T(p) * R(theta) * T(-p)   rotates about pivot p.
#include "transform2d.h"

#include <cmath>

Mat3 Mat3::identity() {
    return Mat3{{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
}

Mat3 Mat3::operator*(const Mat3& o) const {
    Mat3 r{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            r.m[i][j] = 0;
            for (int k = 0; k < 3; ++k) r.m[i][j] += m[i][k] * o.m[k][j];
        }
    return r;
}

Mat3 translate(double tx, double ty) {          // x' = x + tx,  y' = y + ty
    return Mat3{{{1, 0, tx}, {0, 1, ty}, {0, 0, 1}}};
}

Mat3 scale(double sx, double sy) {              // x' = sx*x,  y' = sy*y
    return Mat3{{{sx, 0, 0}, {0, sy, 0}, {0, 0, 1}}};
}

Mat3 rotate(double t) {                         // x' = x cos - y sin,  y' = x sin + y cos
    const double c = std::cos(t), s = std::sin(t);
    return Mat3{{{c, -s, 0}, {s, c, 0}, {0, 0, 1}}};
}

Mat3 rotateAbout(double radians, PointF p) {
    return translate(p.x, p.y) * rotate(radians) * translate(-p.x, -p.y);
}

Mat3 scaleAbout(double sx, double sy, PointF p) {
    return translate(p.x, p.y) * scale(sx, sy) * translate(-p.x, -p.y);
}

Mat3 inverse(const Mat3& a) {
    const auto& m = a.m;
    const double det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
                     - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
                     + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    if (std::fabs(det) < 1e-12) return Mat3::identity();
    const double d = 1.0 / det;
    Mat3 r{};
    r.m[0][0] =  (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * d;
    r.m[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * d;
    r.m[0][2] =  (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * d;
    r.m[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * d;
    r.m[1][1] =  (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * d;
    r.m[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * d;
    r.m[2][0] =  (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * d;
    r.m[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * d;
    r.m[2][2] =  (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * d;
    return r;
}

PointF apply(const Mat3& m, PointF p) {
    return PointF{m.m[0][0] * p.x + m.m[0][1] * p.y + m.m[0][2],
                  m.m[1][0] * p.x + m.m[1][1] * p.y + m.m[1][2]};
}

Polygon apply(const Mat3& m, const Polygon& poly) {
    Polygon out;
    out.vertices.reserve(poly.vertices.size());
    for (const auto& v : poly.vertices) out.vertices.push_back(apply(m, v));
    return out;
}
