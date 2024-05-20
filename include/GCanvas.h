/*
 *  Copyright 2024 Aruj Bansal
 */

#ifndef GCanvas_h_DEFINED
#define GCanvas_h_DEFINED

#include "GRect.h"
#include "GColor.h"
#include "GBitmap.h"
#include "GPath.h"
#include "GPaint.h"
#include "GBlender.h"
#include "GEdge.h"
#include "GMatrix.h"

#include <array>
#include <stack>

using BlendProc = GPixel (*)(GPixel, GPixel);
using BlitzProc = void (*)(int, int, int, const GBitmap &, const GPixel *);

template<bool has_shader>
struct BlitRow {
    template<BlendProc blend_function>
    static void blit_row(int x1, int x2, int y, const GBitmap &device, const GPixel row[]) {
        for (int x = x1; x < x2; x++) {
            GPixel *dst = device.getAddr(x, y);

            if (has_shader) *dst = blend_function(row[x - x1], *dst);
            else *dst = blend_function(row[0], *dst);
        }
    }

    constexpr static const BlitzProc normal_blend[12] = {blit_row<GBlender::kClear>,
                                                         blit_row<GBlender::kSrc>,
                                                         blit_row<GBlender::kDst>,
                                                         blit_row<GBlender::kSrcOver>,
                                                         blit_row<GBlender::kDstOver>,
                                                         blit_row<GBlender::kSrcIn>,
                                                         blit_row<GBlender::kDstIn>,
                                                         blit_row<GBlender::kSrcOut>,
                                                         blit_row<GBlender::kDstOut>,
                                                         blit_row<GBlender::kSrcATop>,
                                                         blit_row<GBlender::kDstATop>,
                                                         blit_row<GBlender::kXor>};

    constexpr static const BlitzProc blend255[12] = {blit_row<GBlender::kClear>,
                                                     blit_row<GBlender::kSrc>,
                                                     blit_row<GBlender::kDst>,
                                                     blit_row<GBlender::kSrc>,
                                                     blit_row<GBlender::kDstOver>,
                                                     blit_row<GBlender::kSrcIn>,
                                                     blit_row<GBlender::kDst>,
                                                     blit_row<GBlender::kSrcOut>,
                                                     blit_row<GBlender::kClear>,
                                                     blit_row<GBlender::kSrcIn>,
                                                     blit_row<GBlender::kDstOver>,
                                                     blit_row<GBlender::kSrcOut>};


    constexpr static const BlitzProc blend0[12] = {blit_row<GBlender::kClear>,
                                                   blit_row<GBlender::kClear>,
                                                   blit_row<GBlender::kDst>,
                                                   blit_row<GBlender::kDst>,
                                                   blit_row<GBlender::kDst>,
                                                   blit_row<GBlender::kClear>,
                                                   blit_row<GBlender::kClear>,
                                                   blit_row<GBlender::kClear>,
                                                   blit_row<GBlender::kDst>,
                                                   blit_row<GBlender::kDst>,
                                                   blit_row<GBlender::kClear>,
                                                   blit_row<GBlender::kDst>};
};

class GCanvas {
public:
    explicit GCanvas(const GBitmap &device) : fDevice(device) {
        transformations.emplace();
    }

    void save();

    void restore();

    void concat(const GMatrix &matrix);

    void clear(const GColor &color);

    void drawRect(const GRect &rect, const GPaint &paint);

    void drawConvexPolygon(const GPoint[], int count, const GPaint &);

    void drawPath(const GPath &, const GPaint &);

    template<bool, int>
    void drawPath(std::vector<Edge> &, const GPaint &);

    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[],
                  const GPaint &);

    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint &);

    // Helpers
    void translate(float x, float y) {
        this->concat(GMatrix::Translate(x, y));
    }

    void scale(float x, float y) {
        this->concat(GMatrix::Scale(x, y));
    }

    void rotate(float radians) {
        this->concat(GMatrix::Rotate(radians));
    }

    // Helpers
    // Note -- these used to be virtuals, but now they are 'demoted' to just methods
    //         that, in turn, call through to the new virtuals. This is done mostly
    //         for compatibility with our old calling code (e.g. pa1 tests).

    void fillRect(const GRect &rect, const GColor &color) {
        this->drawRect(rect, GPaint(color));
    }

public:
    const GBitmap fDevice;
    std::stack<GMatrix> transformations;
};

/**
 *  If the bitmap is valid for drawing into, this returns a subclass that can perform the
 *  drawing. If bitmap is invalid, this returns NULL.
 */
std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap &bitmap);

/**
 *  Implement this, drawing into the provided canvas, and returning the title of your artwork.
 */
std::string GDrawSomething(GCanvas *canvas, GISize dim);

#endif