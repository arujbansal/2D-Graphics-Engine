#ifndef GProxyShader_DEFINED
#define GProxyShader_DEFINED

#include "GBitmapShader.h"

class GProxyShader : public GShader {
public:
    GProxyShader(GShader &, const GMatrix &);

    bool isOpaque() override;

    bool setContext(const GMatrix &ctm) override;

    void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
    GShader *real_shader; // bitmap shader
    GMatrix extra_transformer;
};

inline std::unique_ptr<GShader> GCreateProxyShader(GShader &shader, const GMatrix &transformer) {
    return std::unique_ptr<GShader>(new GProxyShader(shader, transformer));
}

#endif