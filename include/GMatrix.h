#ifndef GMatrix_DEFINED
#define GMatrix_DEFINED

#include "GColor.h"
#include "GMath.h"
#include "GPoint.h"
#include "GRect.h"
#include <optional>

class GMatrix {
public:
    /** [ a  c  e ]         [ 0 2 4 ] <-- indices
     *  [ b  d  f ]         [ 1 3 5 ]
     *  [ 0  0  1 ]  <-- implied, not stored
     */
    GMatrix();

    GMatrix(float a, float c, float e, float b, float d, float f);

    GMatrix(GVector e0, GVector e1, GVector origin);

    GMatrix(const GMatrix &other) = default;

    GVector e0();

    GVector e1();

    GVector origin();

    float operator[](int index) const;

    float &operator[](int index);

    bool operator==(const GMatrix &m);

    bool operator!=(const GMatrix &m);

    static GMatrix Translate(float tx, float ty);

    static GMatrix Scale(float sx, float sy);

    static GMatrix Rotate(float radians);

    /**
     *  Return the product of two matrices: a * b
     */
    static GMatrix Concat(const GMatrix &a, const GMatrix &b);

    /**
     *  If the inverse exists, return it, else return {} to signal no return value.
     */
    [[nodiscard]] std::optional<GMatrix> invert() const;

    /**
     *  Transform the set of points in src, storing the resulting points in dst, by applying this
     *  matrix. It is the caller's responsibility to allocate dst to be at least as large as src.
     *
     *  [ a  c  e ] [ x ]     x' = ax + cy + e
     *  [ b  d  f ] [ y ]     y' = bx + dy + f
     *  [ 0  0  1 ] [ 1 ]
     *
     *  Note: It is legal for src and dst to point to the same memory (however, they may not
     *  partially overlap). Thus the following is supported.
     *
     *  GPoint pts[] = { ... };
     *  matrix.mapPoints(pts, pts, count);
     */
    void mapPoints(GPoint dst[], const GPoint src[], int count) const;

    // These helper methods are implemented in terms of the previous methods.
    friend GMatrix operator*(const GMatrix &a, const GMatrix &b);

    void mapPoints(GPoint pts[], int count) const;

    GPoint operator*(GPoint p) const;

private:
    float fMat[6]{};
};

#endif
