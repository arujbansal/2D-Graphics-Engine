/*
 *  Copyright 2024 Aruj Bansal
 */

#include "../include/GTriangleGradientShader.h"

GTriangleGradientShader::GTriangleGradientShader(const GPoint verts[3], const GColor colors[3]) {
    GPoint vec_u = verts[1] - verts[0];
    GPoint vec_v = verts[2] - verts[0];

    unit_mapper = GMatrix(vec_u.x, vec_v.x, verts[0].x,
                          vec_u.y, vec_v.y, verts[0].y);

    color0 = colors[0];
    diff_color1 = colors[1] - colors[0];
    diff_color2 = colors[2] - colors[0];
    my_colors = {colors[0], colors[1], colors[2]};
}

bool GTriangleGradientShader::isOpaque() {
    return false;
}

bool GTriangleGradientShader::setContext(const GMatrix &ctm) {
    inv = (ctm * unit_mapper).invert();
    return inv.has_value();
}

void GTriangleGradientShader::shadeRow(int x, int y, int count, GPixel *row) {
    GPoint p = inv.value() * GPoint{(float) x + 0.5f, (float) y + 0.5f};

    GColor diff_color = inv.value()[0] * diff_color1 + inv.value()[1] * diff_color2;
    GColor cur_color = p.x * diff_color1 + p.y * diff_color2 + color0;

    row[0] = gutils::premul_255_clamp(cur_color);
    cur_color += diff_color;

    for (int i = 1; i < count - 1; ++i) {
        row[i] = gutils::premul_255(cur_color);
        cur_color += diff_color;
    }

    row[count - 1] = gutils::premul_255_clamp(cur_color);
}
