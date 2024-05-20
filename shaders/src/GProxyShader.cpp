/*
 *  Copyright 2024 Aruj Bansal
 */

#include "../include/GProxyShader.h"

#include <utility>

GProxyShader::GProxyShader(GShader &shader, const GMatrix &transformer) : real_shader(&shader),
                                                                           extra_transformer(transformer) {}

bool GProxyShader::isOpaque() {
    return real_shader->isOpaque();
}

bool GProxyShader::setContext(const GMatrix &ctm) {
    return real_shader->setContext(ctm * extra_transformer);
}

void GProxyShader::shadeRow(int x, int y, int count, GPixel *row) {
    real_shader->shadeRow(x, y, count, row);
}
