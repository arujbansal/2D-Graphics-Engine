/*
 * Copyright 2024 Aruj Bansal
 */

#include "../include/GBitmapShader.h"

MyShader::MyShader(const GBitmap &device, const GMatrix &localMatrix, GTileMode mode) {
    localBitmap = device;
    this->localMatrix = localMatrix;

    switch (mode) {
        case GTileMode::kClamp:
            tiler = tile_clamp;
            break;
        case GTileMode::kRepeat:
            tiler = tile_repeat;
            break;
        case GTileMode::kMirror:
            tiler = tile_mirror;
            break;
    }
}

bool MyShader::isOpaque() {
    return localBitmap.isOpaque();
}

bool MyShader::setContext(const GMatrix &ctm) {
    inv = (ctm * localMatrix).invert();
    if (!inv.has_value()) return false;

    inv = inv.value();
    return true;
}

void MyShader::shadeRow(int x, int y, int count, GPixel row[]) {
    auto [inv_x, inv_y] = inv.value() * GPoint{(float) x + 0.5f, (float) y + 0.5f};

    for (int i = 0; i < count; ++i) {
        auto [clamped_x, clamped_y] = tiler(GFloorToInt(inv_x), GFloorToInt(inv_y), localBitmap.width(),
                                            localBitmap.height());

        row[i] = *localBitmap.getAddr(clamped_x, clamped_y);

        ++x;
        inv_x += inv.value()[0];
        inv_y += inv.value()[1];
    }
}

std::pair<int, int> MyShader::tile_clamp(int x, int y, int width, int height) {
    return std::make_pair(std::max(0, std::min(x, width - 1)),
                          std::max(0, std::min(y, height - 1)));
}

std::pair<int, int> MyShader::tile_repeat(int x, int y, int width, int height) {
    x %= width;
    y %= height;

    x += width;
    y += height;

    if (x >= width) x -= width;
    if (y >= height) y -= height;

    return std::make_pair(x, y);
}

std::pair<int, int> MyShader::tile_mirror(int x, int y, int width, int height) {
    x %= 2 * width;
    y %= 2 * height;

    if (x < 0) x += 2 * width;
    if (y < 0) y += 2 * height;

    if (x >= width) x = 2 * width - x - 1;
    if (y >= height) y = 2 * height - y - 1;

    return std::make_pair(x, y);
}

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap &device, const GMatrix &localMatrix, GTileMode mode) {
    return std::unique_ptr<GShader>(new MyShader(device, localMatrix, mode));
}
