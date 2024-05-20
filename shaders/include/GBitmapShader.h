#ifndef GBitmapShader_h_DEFINED
#define GBitmapShader_h_DEFINED

#include "GShader.h"
#include "../../include/GMatrix.h"
#include "../../include/GBitmap.h"

using TileProc = std::pair<int, int> (*)(int, int, int, int);

class MyShader : public GShader {
public:
    MyShader(const GBitmap &device, const GMatrix &localMatrix, GTileMode mode);

    bool isOpaque() override;

    bool setContext(const GMatrix &ctm) override;

    void shadeRow(int x, int y, int count, GPixel row[]) override;

    static std::pair<int, int> tile_clamp(int x, int y, int width, int height);

    static std::pair<int, int> tile_repeat(int x, int y, int width, int height);

    static std::pair<int, int> tile_mirror(int x, int y, int width, int height);


private:
    GMatrix localMatrix;
    std::optional<GMatrix> inv;
    GBitmap localBitmap;
    TileProc tiler{};
};

#endif
