#ifndef GBlender_DEFINED
#define GBlender_DEFINED

#include "GColor.h"
#include "GPixel.h"
#include "GBlendMode.h"
#include <array>

namespace GBlender {
    static int32_t divBy255(const int32_t prod) {
        return (prod + 128) * 257 >> 16;
    }

    static GPixel kClear(GPixel src, GPixel dst) {
        return 0;
    }

    static GPixel kSrc(GPixel src, GPixel dst) {
        return src;
    }

    static GPixel kDst(GPixel src, GPixel dst) {
        return dst;
    }

    static GPixel kSrcOver(GPixel src, GPixel dst) {
        std::array<int, 4> srcValues = {GPixel_GetA(src), GPixel_GetR(src), GPixel_GetG(src), GPixel_GetB(src)};
        std::array<int, 4> dstValues = {GPixel_GetA(dst), GPixel_GetR(dst), GPixel_GetG(dst), GPixel_GetB(dst)};

        for (int channel = 3; channel >= 0; --channel)
            srcValues[channel] += divBy255((255 - srcValues[0]) * dstValues[channel]);

        return GPixel_PackARGB(srcValues[0], srcValues[1], srcValues[2], srcValues[3]);
    }

    static GPixel kDstOver(GPixel src, GPixel dst) {
        std::array<int, 4> srcValues = {GPixel_GetA(src), GPixel_GetR(src), GPixel_GetG(src), GPixel_GetB(src)};
        std::array<int, 4> dstValues = {GPixel_GetA(dst), GPixel_GetR(dst), GPixel_GetG(dst), GPixel_GetB(dst)};

        for (int channel = 3; channel >= 0; --channel)
            srcValues[channel] = dstValues[channel] + divBy255((255 - dstValues[0]) * srcValues[channel]);

        return GPixel_PackARGB(srcValues[0], srcValues[1], srcValues[2], srcValues[3]);
    }

    static GPixel kSrcIn(GPixel src, GPixel dst) {
        std::array<int, 4> srcValues = {GPixel_GetA(src), GPixel_GetR(src), GPixel_GetG(src), GPixel_GetB(src)};
        int dst_a = GPixel_GetA(dst);

        for (int channel = 3; channel >= 0; --channel)
            srcValues[channel] = divBy255(srcValues[channel] * dst_a);

        return GPixel_PackARGB(srcValues[0], srcValues[1], srcValues[2], srcValues[3]);
    }

    static GPixel kDstIn(GPixel src, GPixel dst) {
        int src_a = GPixel_GetA(src);
        std::array<int, 4> dstValues = {GPixel_GetA(dst), GPixel_GetR(dst), GPixel_GetG(dst), GPixel_GetB(dst)};

        for (int channel = 3; channel >= 0; --channel)
            dstValues[channel] = divBy255(dstValues[channel] * src_a);

        return GPixel_PackARGB(dstValues[0], dstValues[1], dstValues[2], dstValues[3]);
    }

    static GPixel kSrcOut(GPixel src, GPixel dst) {
        std::array<int, 4> srcValues = {GPixel_GetA(src), GPixel_GetR(src), GPixel_GetG(src), GPixel_GetB(src)};
        int dst_a = GPixel_GetA(dst);

        for (int channel = 3; channel >= 0; --channel)
            srcValues[channel] = divBy255((255 - dst_a) * srcValues[channel]);

        return GPixel_PackARGB(srcValues[0], srcValues[1], srcValues[2], srcValues[3]);
    }

    static GPixel kDstOut(GPixel src, GPixel dst) {
        int src_a = GPixel_GetA(src);
        std::array<int, 4> dstValues = {GPixel_GetA(dst), GPixel_GetR(dst), GPixel_GetG(dst), GPixel_GetB(dst)};

        for (int channel = 3; channel >= 0; --channel)
            dstValues[channel] = divBy255((255 - src_a) * dstValues[channel]);

        return GPixel_PackARGB(dstValues[0], dstValues[1], dstValues[2], dstValues[3]);
    }

    static GPixel kSrcATop(GPixel src, GPixel dst) {
        std::array<int, 4> srcValues = {GPixel_GetA(src), GPixel_GetR(src), GPixel_GetG(src), GPixel_GetB(src)};
        std::array<int, 4> dstValues = {GPixel_GetA(dst), GPixel_GetR(dst), GPixel_GetG(dst), GPixel_GetB(dst)};

        for (int channel = 3; channel >= 0; --channel)
            dstValues[channel] =
                    divBy255((255 - srcValues[0]) * dstValues[channel]) + divBy255(srcValues[channel] * dstValues[0]);

        return GPixel_PackARGB(dstValues[0], dstValues[1], dstValues[2], dstValues[3]);
    }

    static GPixel kDstATop(GPixel src, GPixel dst) {
        std::array<int, 4> srcValues = {GPixel_GetA(src), GPixel_GetR(src), GPixel_GetG(src), GPixel_GetB(src)};
        std::array<int, 4> dstValues = {GPixel_GetA(dst), GPixel_GetR(dst), GPixel_GetG(dst), GPixel_GetB(dst)};

        for (int channel = 3; channel >= 0; --channel)
            srcValues[channel] =
                    divBy255((255 - dstValues[0]) * srcValues[channel]) + divBy255(dstValues[channel] * srcValues[0]);

        return GPixel_PackARGB(srcValues[0], srcValues[1], srcValues[2], srcValues[3]);
    }

    static GPixel kXor(GPixel src, GPixel dst) {
        std::array<int, 4> srcValues = {GPixel_GetA(src), GPixel_GetR(src), GPixel_GetG(src), GPixel_GetB(src)};
        std::array<int, 4> dstValues = {GPixel_GetA(dst), GPixel_GetR(dst), GPixel_GetG(dst), GPixel_GetB(dst)};

        for (int channel = 3; channel >= 0; --channel)
            srcValues[channel] =
                    divBy255((255 - dstValues[0]) * srcValues[channel]) +
                    divBy255((255 - srcValues[0]) * dstValues[channel]);

        return GPixel_PackARGB(srcValues[0], srcValues[1], srcValues[2], srcValues[3]);
    }

//    static GPixel kSrcOverT(GPixel src, GPixel dst) {
//        std::array<int, 4> srcValues = {GPixel_GetA(src), GPixel_GetR(src), GPixel_GetG(src), GPixel_GetB(src)};
//        std::array<int, 4> dstValues = {GPixel_GetA(dst), GPixel_GetR(dst), GPixel_GetG(dst), GPixel_GetB(dst)};
//
//        for (int channel = 3; channel >= 0; --channel)
//            dstValues[channel] = srcValues[channel] + dstValues[channel];
//
//        return GPixel_PackARGB(dstValues[0], dstValues[1], dstValues[2], dstValues[3]);
//    }
//
//    static GPixel kSrcATopT(GPixel src, GPixel dst) {
//        std::array<int, 4> srcValues = {GPixel_GetA(src), GPixel_GetR(src), GPixel_GetG(src), GPixel_GetB(src)};
//        std::array<int, 4> dstValues = {GPixel_GetA(dst), GPixel_GetR(dst), GPixel_GetG(dst), GPixel_GetB(dst)};
//
//        for (int channel = 3; channel >= 0; --channel)
//            dstValues[channel] =
//                    dstValues[channel] + divBy255(srcValues[channel] * dstValues[0]);
//
//        return GPixel_PackARGB(dstValues[0], dstValues[1], dstValues[2], dstValues[3]);
//    }
}

#endif
