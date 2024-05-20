#ifndef GUtils_h_DEFINED
#define GUtils_h_DEFINED

#include "GPoint.h"

namespace gutils {
    inline std::pair<float, float> line_properties_x(const GPoint p1, const GPoint p2) {
        float slope_x = (p1.x - p2.x) / (p1.y - p2.y);
        float x_intercept = p2.x - p2.y * slope_x;
        return std::make_pair(slope_x, x_intercept);
    }

    inline std::pair<float, float> line_properties_y(const GPoint p1, const GPoint p2) {
        float slope_y = (p1.y - p2.y) / (p1.x - p2.x);
        float y_intercept = p2.y - p2.x * slope_y;
        return std::make_pair(slope_y, y_intercept);
    }

    inline float line_intercept_x(const GPoint p1, const GPoint p2, float slope) {
        return p2.x - p2.y * slope;
    }

    inline float line_intercept_y(const GPoint p1, const GPoint p2, float slope) {
        return p2.y - p2.x * slope;
    }

    inline float query_x(float y, float slope_x, float intercept_x) {
        return y * slope_x + intercept_x;
    }

    inline float query_y(float x, float slope_y, float intercept_y) {
        return x * slope_y + intercept_y;
    }

    inline bool is_inside(int y, int top_y, int bottom_y) {
        return y >= top_y && y < bottom_y;
    }

    inline int premul_255_float(float x) {
        return GRoundToInt(x * 255);
    }

    inline GPixel premul_255(const GColor color) {
        return GPixel_PackARGB(premul_255_float(color.a), premul_255_float(color.a * color.r),
                               premul_255_float(color.a * color.g),
                               premul_255_float(color.a * color.b));
    }

    inline GPixel premul_255_clamp(const GColor color) {
        float a = std::max(0.0f, std::min(1.0f, color.a));
        float r = std::max(0.0f, std::min(1.0f, color.a * color.r));
        float g = std::max(0.0f, std::min(1.0f, color.a * color.g));
        float b = std::max(0.0f, std::min(1.0f, color.a * color.b));

        return GPixel_PackARGB(premul_255_float(a),
                               premul_255_float(r),
                               premul_255_float(g),
                               premul_255_float(b));
    }

    inline int mapUnit255(float x) { return GRoundToInt(x * 255); }

    /*
     * Maps RGB values from [0.0, 1.0] to [0, 255] and pre-multiplies with alpha.
     */
    inline GPixel pixelizeFloatColor(const GColor color) {
        // Create a single integer out of the 4 channels. each channel corresponding to one byte
        return GPixel_PackARGB(mapUnit255(color.a), mapUnit255(color.a * color.r), mapUnit255(color.a * color.g),
                               mapUnit255(color.a * color.b));
    }

    inline GMatrix compute_triangle_basis(GPoint verts[3]) {
        GPoint vec_u = verts[1] - verts[0];
        GPoint vec_v = verts[2] - verts[0];

        return {vec_u.x, vec_v.x, verts[0].x,
                vec_u.y, vec_v.y, verts[0].y};
    }

    inline int32_t divBy255(const int32_t prod) {
        return (prod + 128) * 257 >> 16;
    }
};

#endif