/*
 *  Copyright 2024 Aruj Bansal
 */

#ifndef GComposeShader_DEFINED
#define GComposeShader_DEFINED

#include "GTriangleGradientShader.h"
#include "GProxyShader.h"

class GComposeShader : public GShader {
public:
    GComposeShader(GShader &, GShader &);

    bool isOpaque() override;

    bool setContext(const GMatrix &ctm) override;

    void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
    GShader *gradient_shader;
    GShader *proxy_shader;
};

inline std::unique_ptr<GShader> GCreateTriangleCompose(GShader &gradient, GShader &proxy) {
    return std::unique_ptr<GShader>(new GComposeShader(gradient, proxy));
}

#endif