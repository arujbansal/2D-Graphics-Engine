#ifndef GBezier_DEFINED
#define GBezier_DEFINED

#include "GPoint.h"

namespace bezier {
    inline GPoint eval_quad(float t, GPoint points[3]) {
        return (points[0] * (1 - t) * (1 - t)) + (2 * points[1] * t * (1 - t)) + (points[2] * t * t);
    }

    inline GPoint eval_cubic(float t, GPoint points[4]) {
        return (points[0] * (1 - t) * (1 - t) * (1 - t)) + (3 * points[1] * t * (1 - t) * (1 - t)) +
               (3 * points[2] * (1 - t) * t * t) + (points[3] * t * t * t);
    }

    inline float derivative_zero_quad(float a, float b, float c) {
        float denominator = c - 2 * b + a;

        if (denominator == 0)
            return -1.0f;

        float numerator = a - b;
        return numerator / denominator;
    }

    inline std::pair<float, float> derivative_zero_cubic(float a, float b, float c, float d) {
        float term3 = (d - a + 3 * b - 3 * c);
        if (term3 == 0)
            return {-1.0f, -1.0f};

        float term1 = (-1 * a + 2 * b - c);
        float term2 = std::sqrt(b * b - d * b - b * c + d * a + c * c - a * c);

        float t1 = (term1 + term2) / term3;
        float t2 = (term1 - term2) / term3;

        return {t1, t2};

    }

    inline void chop_quad_at(const GPoint src[3], GPoint dst[5], float t) {
        dst[0] = src[0];
        dst[4] = src[2];

        dst[1] = (1 - t) * src[0] + t * src[1];
        dst[3] = (1 - t) * src[1] + t * src[2];

        dst[2] = (1 - t) * dst[1] + t * dst[3];
    }

    inline void chop_cubic_at(const GPoint src[4], GPoint dst[7], float t) {
        dst[0] = src[0];
        dst[6] = src[3];

        dst[1] = (1 - t) * src[0] + t * src[1];
        dst[5] = (1 - t) * src[2] + t * src[3];


        GPoint temp = (1 - t) * src[1] + t * src[2];
        dst[2] = (1 - t) * dst[1] + t * temp;
        dst[4] = (1 - t) * temp + t * dst[5];

        dst[3] = (1 - t) * dst[2] + t * dst[4];
    }
};

#endif