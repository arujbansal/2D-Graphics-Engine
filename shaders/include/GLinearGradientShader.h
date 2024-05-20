#ifndef GLinearGradientShader_h_DEFINED
#define GLinearGradientShader_h_DEFINED

#include "GShader.h"
#include "../../include/GMatrix.h"
#include "../../include/GBitmap.h"
#include "../../include/GUtils.h"

using GradientTileProc = std::pair<int, float> (*)(float, int);


class GLinearGradientShader : public GShader {
public:
    GLinearGradientShader(GPoint, GPoint, const GColor[], int, GTileMode mode);

    bool isOpaque() override;

    bool setContext(const GMatrix &ctm) override;

    void shadeRow(int x, int y, int count, GPixel row[]) override;

    template<bool>
    void shadeRow(int x, int y, int count, GPixel row[]);

    static std::pair<int, float> tile_repeat(float x, int num_colors);

    static std::pair<int, float> tile_mirror(float x, int num_colors);

private:
    GMatrix line_mapper;
    std::optional<GMatrix> inv;
    std::vector<GColor> colors, colors_diff;
    std::pair<GPixel, GPixel> premul_ends;
    int num_colors;
    GTileMode tile_mode;
    GradientTileProc tiler{};
};

#endif


