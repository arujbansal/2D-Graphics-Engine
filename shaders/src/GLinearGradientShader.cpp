/*
 *  Copyright 2024 Aruj Bansal
 */

#include "../include/GLinearGradientShader.h"

GLinearGradientShader::GLinearGradientShader(GPoint p0, GPoint p1, const GColor _colors[], int count, GTileMode mode) {
    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;

    tile_mode = mode;

    switch (mode) {
        case GTileMode::kClamp:
            break;
        case GTileMode::kRepeat:
            tiler = tile_repeat;
            break;
        case GTileMode::kMirror:
            tiler = tile_mirror;
            break;
    }

    line_mapper = {dx, -dy, p0.x,
                   dy, dx, p0.y};

    num_colors = count;

    premul_ends.first = gutils::premul_255(_colors[0]);
    premul_ends.second = gutils::premul_255(_colors[count - 1]);

    if (num_colors == 1) return;

    colors.resize(count);
    for (int i = 0; i < count; i++)
        colors[i] = _colors[i];

    colors_diff.resize(count);
    for (int i = 0; i < count - 1; i++) {
        colors_diff[i].r = _colors[i + 1].r - _colors[i].r;
        colors_diff[i].g = _colors[i + 1].g - _colors[i].g;
        colors_diff[i].b = _colors[i + 1].b - _colors[i].b;
        colors_diff[i].a = _colors[i + 1].a - _colors[i].a;
    }
}

bool GLinearGradientShader::isOpaque() {
    return false;
}

bool GLinearGradientShader::setContext(const GMatrix &ctm) {
    inv = (ctm * line_mapper).invert();
    return inv.has_value();
}

void GLinearGradientShader::shadeRow(int x, int y, int count, GPixel row[]) {
    switch (num_colors) {
        case 1:
            std::fill(row, row + count, premul_ends.first);
            break;
        case 2:
            shadeRow<true>(x, y, count, row);
            break;
        default:
            shadeRow<false>(x, y, count, row);
    }
}

std::pair<int, float> GLinearGradientShader::tile_repeat(float x, int num_colors) {
    x = x - floor(x); // Now x is in between [0, 1] and we are ready to scale.

    float scaled_x = x * (float) (num_colors - 1);
    int floored_x = GFloorToInt(scaled_x);
    float dist = (scaled_x - (float) floored_x);

    return std::make_pair(abs(floored_x), dist);
}

std::pair<int, float> GLinearGradientShader::tile_mirror(float x, int num_colors) {
    x *= 0.5f;
    x = x - floor(x);
    if (x > 0.5f)
        x = 1 - x;

    x *= 2;

    float scaled_x = x * (float) (num_colors - 1);
    int floored_x = GFloorToInt(scaled_x);
    float dist = (scaled_x - (float) floored_x);

    return std::make_pair(abs(floored_x), dist);
}

template<bool two_colors>
void GLinearGradientShader::shadeRow(int x, int y, int count, GPixel row[]) {
    // Map param x onto the x-axis to make computation easier. We do matrix multiplication but on a "single cell" matrix
    float inv_x = inv.value()[0] * ((float) x + 0.5f) + inv.value()[2] * ((float) y + 0.5f) + inv.value()[4];

    for (int i = 0; i < count; ++i) {
        bool smaller = inv_x <= 0.0f;
        bool greater = inv_x >= 1.0f;

        if (smaller && tile_mode == GTileMode::kClamp) {
            row[i] = premul_ends.first;
        } else if (greater && tile_mode == GTileMode::kClamp) {
            row[i] = premul_ends.second;
        } else {
            float scaled_x, dist;
            int floored_x;

            if (smaller || greater) {
                std::pair<int, float> res = tiler(inv_x, num_colors);
                floored_x = res.first;
                dist = res.second;
            } else {
                if (two_colors) {
                    floored_x = 0;
                    dist = inv_x;
                } else {
                    scaled_x = inv_x * (float) (num_colors - 1);
                    floored_x = GFloorToInt(scaled_x);
                    dist = (scaled_x - (float) floored_x);
                }
            }

            GColor pixel_color{colors[floored_x].r + dist * colors_diff[floored_x].r,
                               colors[floored_x].g + dist * colors_diff[floored_x].g,
                               colors[floored_x].b + dist * colors_diff[floored_x].b,
                               colors[floored_x].a + dist * colors_diff[floored_x].a};

            row[i] = gutils::premul_255(pixel_color);
        }

        inv_x += inv.value()[0];
        ++x;
    }
}

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor color[], int count, GTileMode mode) {
    if (count < 1) return nullptr;
    return std::unique_ptr<GShader>(new GLinearGradientShader(p0, p1, color, count, mode));
}
