#ifndef GTriangleGradientShader_DEFINED
#define GTriangleGradientShader_DEFINED

#include "GShader.h"
#include "../../include/GMatrix.h"
#include "../../include/GBitmap.h"
#include "../../include/GUtils.h"

class GTriangleGradientShader : public GShader {
public:
    GTriangleGradientShader(const GPoint verts[3], const GColor colors[3]);

    bool isOpaque() override;

    bool setContext(const GMatrix &ctm) override;

    void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
    GMatrix unit_mapper;
    std::optional<GMatrix> inv;
    std::vector<GColor> my_colors;
    GColor color0, diff_color1, diff_color2;
};

inline std::unique_ptr<GShader> GCreateTriangleGradient(const GPoint verts[3], const GColor colors[3]) {
    return std::unique_ptr<GShader>(new GTriangleGradientShader(verts, colors));
}

#endif