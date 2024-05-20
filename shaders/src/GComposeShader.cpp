#include "../include/GComposeShader.h"

GComposeShader::GComposeShader(GShader &gradient, GShader &proxy) : gradient_shader(&gradient), proxy_shader(&proxy) {}

bool GComposeShader::isOpaque() {
    return gradient_shader->isOpaque() && proxy_shader->isOpaque();
}

bool GComposeShader::setContext(const GMatrix &ctm) {
    return gradient_shader->setContext(ctm) && proxy_shader->setContext(ctm);
}

void GComposeShader::shadeRow(int x, int y, int count, GPixel *row) {
    GPixel gradient_row[count], proxy_row[count];

    gradient_shader->shadeRow(x, y, count, gradient_row);
    proxy_shader->shadeRow(x, y, count, proxy_row);

    for (int i = 0; i < count; ++i) {
        row[i] = GPixel_PackARGB(gutils::divBy255(GPixel_GetA(gradient_row[i]) * GPixel_GetA(proxy_row[i])),
                                 gutils::divBy255(GPixel_GetR(gradient_row[i]) * GPixel_GetR(proxy_row[i])),
                                 gutils::divBy255(GPixel_GetG(gradient_row[i]) * GPixel_GetG(proxy_row[i])),
                                 gutils::divBy255(GPixel_GetB(gradient_row[i]) * GPixel_GetB(proxy_row[i])));
    }
}
