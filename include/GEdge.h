/*
 *  Copyright 2024 Aruj Bansal
 */

#ifndef GEdge_h_DEFINED
#define GEdge_h_DEFINED

#include "GPoint.h"
#include "GUtils.h"

struct Edge {
    int top, bottom, winding;
    float cur_x_val;
    float slope_x, intercept_x;
    GPoint tp1, tp2;

    Edge(const GPoint &p1, const GPoint &p2, int _winding = 0) {
        tp1 = p1;
        tp2 = p2;
        winding = _winding;

        compute_x_properties();

        int p1_y = GRoundToInt(p1.y);
        int p2_y = GRoundToInt(p2.y);

        top = std::min(p1_y, p2_y);
        bottom = std::max(p1_y, p2_y);

        cur_x_val = ((float) top - 0.5f) * slope_x + intercept_x;

    }

    void compute_x_properties() {
        std::tie(slope_x, intercept_x) = gutils::line_properties_x(tp1, tp2);
    }

    void set_x_properties(float _slope_x, float _intercept_x) {
        slope_x = _slope_x;
        intercept_x = _intercept_x;
    }

    float query_x(float y) {
        return cur_x_val += slope_x;
    }

    int query_x_round(float y) {
        return GRoundToInt(query_x(y));
    }
};

#endif