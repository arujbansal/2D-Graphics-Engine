#include "../include/GMatrix.h"
#include <cmath>

GMatrix::GMatrix(float a, float c, float e, float b, float d, float f) {
    fMat[0] = a; fMat[2] = c; fMat[4] = e;
    fMat[1] = b; fMat[3] = d; fMat[5] = f;
}

GMatrix::GMatrix(GVector e0, GVector e1, GVector origin) {
    fMat[0] = e0.x; fMat[2] = e1.x; fMat[4] = origin.x;
    fMat[1] = e0.y; fMat[3] = e1.y; fMat[5] = origin.y;
}

GVector GMatrix::e0() { return {fMat[0], fMat[1]}; }

GVector GMatrix::e1() { return {fMat[2], fMat[3]}; }

GVector GMatrix::origin() { return {fMat[4], fMat[5]}; }

float GMatrix::operator[](int index) const {
    assert(index >= 0 && index < 6);
    return fMat[index];
}

float &GMatrix::operator[](int index) {
    assert(index >= 0 && index < 6);
    return fMat[index];
}

bool GMatrix::operator==(const GMatrix &m) {
    for (int i = 0; i < 6; ++i) {
        if (fMat[i] != m.fMat[i]) {
            return false;
        }
    }
    return true;
}

bool GMatrix::operator!=(const GMatrix &m) { return !(*this == m); }

GMatrix operator*(const GMatrix &a, const GMatrix &b) {
    return GMatrix::Concat(a, b);
}

void GMatrix::mapPoints(GPoint pts[], int count) const {
    this->mapPoints(pts, pts, count);
}

GPoint GMatrix::operator*(GPoint p) const {
    this->mapPoints(&p, 1);
    return p;
}

GMatrix::GMatrix() {
    fMat[1] = fMat[2] = fMat[4] = fMat[5] = 0;
    fMat[0] = fMat[3] = 1;
}

GMatrix GMatrix::Translate(float tx, float ty) {
    return {1.0f, 0.0f, tx,
            0.0f, 1.0f, ty};
}

GMatrix GMatrix::Scale(float sx, float sy) {
    return {sx, 0.0f, 0.0f,
            0.0f, sy, 0.0f};
}

GMatrix GMatrix::Rotate(float radians) {
    float sine = std::sin(radians);
    float cosine = std::cos(radians);

    return {cosine, -sine, 0.0f,
            sine, cosine, 0.0f};
}

GMatrix GMatrix::Concat(const GMatrix &a, const GMatrix &b) {
    return {a[0] * b[0] + a[2] * b[1], a[0] * b[2] + a[2] * b[3], a[0] * b[4] + a[2] * b[5] + a[4],
            a[1] * b[0] + a[3] * b[1], a[1] * b[2] + a[3] * b[3], a[1] * b[4] + a[3] * b[5] + a[5]};
}

std::optional<GMatrix> GMatrix::invert() const {
    float det = fMat[0] * fMat[3] - fMat[1] * fMat[2];
    if (det == 0) return {}; // Matrix singular and not invertible

    float k = 1.0f / det;
    return GMatrix(k * fMat[3], k * -fMat[2], k * (fMat[2] * fMat[5] - fMat[3] * fMat[4]),
                   k * -fMat[1], k * fMat[0], k * (fMat[1] * fMat[4] - fMat[0] * fMat[5]));
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; ++i) {
        float x = src[i].x;
        float y = src[i].y;

        dst[i] = GPoint{fMat[0] * x + fMat[2] * y + fMat[4],
                        fMat[1] * x + fMat[3] * y + fMat[5]};
    }
}