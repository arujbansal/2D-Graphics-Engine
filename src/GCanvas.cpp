/*
 *  Copyright 2024 Aruj Bansal
 */

#include "../include/GCanvas.h"
#include "../include/GEdge.h"
#include <vector>
#include <iostream>
#include "../include/GBlender.h"
#include "../shaders/include/GBitmapShader.h"
#include "../include/GPath.h"
#include "../include/GUtils.h"
#include "../shaders/include/GLinearGradientShader.h"
#include "../shaders/include/GTriangleGradientShader.h"
#include "../shaders/include/GComposeShader.h"
#include "../shaders/include/GProxyShader.h"
#include "../include/GBezier.h"
#include <numeric>

void GCanvas::save() {
    transformations.push(transformations.top());
}

void GCanvas::restore() {
    transformations.pop();
}

void GCanvas::concat(const GMatrix &matrix) {
    GMatrix proc = GMatrix::Concat(transformations.top(), matrix);
    transformations.pop();
    transformations.push(proc);
}

void GCanvas::clear(const GColor &color) {
    GPixel pix = gutils::pixelizeFloatColor(color);
    int h = fDevice.height(), w = fDevice.width();

    for (int y = 0; y < h; ++y) {
        GPixel *pixel = fDevice.getAddr(0, y);
        std::fill_n(pixel, w, pix);
    }
}

void clip(const std::vector<std::pair<GPoint, GPoint>> &edges, std::vector<Edge> &clipped, int height,
          int width) {

    int count = (int) edges.size();
    clipped.reserve(4 * count);

    for (int i = 0; i < count; i++) {
        GPoint p1 = edges[i].first;
        GPoint p2 = edges[i].second;

        // Skip edge: If it's horizontal
        if (GRoundToInt(p1.y) == GRoundToInt(p2.y))
            continue;

        int orientation = (p1.y > p2.y ? 1 : -1);
        float slope_x, slope_y, intercept_x, intercept_y;

        // ----------------------------------------------------

        // < VERTICAL CLIPPING

        // Enforce invariant: p1 is always the topmost point of the edge
        if (p1.y > p2.y)
            std::swap(p1, p2);

        // Skip edge: If it lies completely above or below display
        if ((int) p2.y <= 0 || (int) p1.y >= height) continue;

        std::tie(slope_x, intercept_x) = gutils::line_properties_x(p1, p2);

        // New clipped top point
        float clipped_y1 = std::max(0.0f, p1.y);
        p1 = {gutils::query_x(clipped_y1, slope_x, intercept_x), clipped_y1};

        // New clipped top point
        float clipped_y2 = std::min((float) height, p2.y);
        p2 = {gutils::query_x(clipped_y2, slope_x, intercept_x), clipped_y2};

        // <\ VERTICAL CLIPPING

        // ----------------------------------------------------

        // < HORIZONTAL CLIPPING

        // Enforce invariant: p1 is always leftmost point of the edge
        if (p1.x > p2.x)
            std::swap(p1, p2);

        float f_width = (float) width;

        std::tie(slope_y, intercept_y) = gutils::line_properties_y(p1, p2);

        if (p2.x <= 0) { // Edge lies outside the display, to the left
            p1 = {0.0f, p1.y};
            p2 = {0.0f, p2.y};

            clipped.emplace_back(p1, p2, orientation);
        } else if (p1.x >= f_width) { // Edge lies outside the display, to the right
            p1 = {f_width, p1.y};
            p2 = {f_width, p2.y};

            clipped.emplace_back(p1, p2, orientation);
        } else if (p1.x < 0.0f && p2.y > f_width) { // Edge fully intersects display, both ends lie outside
            GPoint left_boundary{0.0f, p1.y};
            GPoint right_boundary{f_width, p2.y};

            GPoint clip_left = GPoint{0.0f, gutils::query_y(0.0f, slope_y, intercept_y)};
            GPoint clip_right = GPoint{f_width, gutils::query_y(f_width, slope_y, intercept_y)};

            clipped.emplace_back(left_boundary, clip_left, orientation);
            clipped.emplace_back(right_boundary, clip_right, orientation);
            clipped.emplace_back(clip_left, clip_right, orientation);
        } else if (p1.x < 0.0f) { // Left end out of canvas
            GPoint left_boundary{0.0f, p1.y};
            GPoint clip_left = GPoint{0.0f, gutils::query_y(0.0f, slope_y, intercept_y)};

            clipped.emplace_back(left_boundary, clip_left, orientation);
            clipped.emplace_back(clip_left, p2, orientation);
        } else if (p2.x > f_width) { // Right end out of canvas
            GPoint right_boundary{f_width, p2.y};
            GPoint clip_right = GPoint{f_width, gutils::query_y(f_width, slope_y, intercept_y)};

            clipped.emplace_back(right_boundary, clip_right, orientation);
            clipped.emplace_back(p1, clip_right, orientation);
        } else if (p1.x >= 0.0f && p2.x <= f_width) { // Both ends in canvas
            clipped.emplace_back(p1, p2, orientation);
        }
        // <\ HORIZONTAL CLIPPING
    }
}

void GCanvas::drawRect(const GRect &rect, const GPaint &paint) {
    GPoint vertices[4] = {{rect.left,  rect.top},
                          {rect.right, rect.top},
                          {rect.right, rect.bottom},
                          {rect.left,  rect.bottom}};

    drawConvexPolygon(vertices, 4, paint);
}

void GCanvas::drawConvexPolygon(const GPoint *vertices, int count, const GPaint &paint) {
    if (count < 2) return;

    GPoint new_vertices[count];

    for (int i = 0; i < count; i++)
        new_vertices[i] = transformations.top() * vertices[i];

    std::vector<std::pair<GPoint, GPoint>> init_edges;
    init_edges.reserve(count + 1);

    for (int i = 0; i < count; i++)
        init_edges.emplace_back(new_vertices[i], new_vertices[(i + 1) % count]);

    std::vector<Edge> clipped;
    clip(init_edges, clipped, fDevice.height(), fDevice.width());
    std::sort(clipped.begin(), clipped.end(), [](const Edge &e1, const Edge &e2) {
        return e1.top < e2.top;
    });

    if ((int) clipped.size() < 2) return;

    int mn = clipped.front().top;
    int mx = clipped.back().bottom;

    std::pair<int, int> row_bounds[mx - mn + 1];
    int edge_1 = 0, edge_2 = 1;

    for (int y = mn; y < mx; y++) {
        if (y >= clipped[edge_1].bottom)
            edge_1 = std::max(edge_1, edge_2) + 1;

        if (y >= clipped[edge_2].bottom)
            edge_2 = std::max(edge_1, edge_2) + 1;

        float laser = (float) y + 0.5f;

        int q1 = clipped[edge_1].query_x_round(laser);
        int q2 = clipped[edge_2].query_x_round(laser);

        if (q1 > q2)
            std::swap(q1, q2);

        row_bounds[y - mn].first = q1;
        row_bounds[y - mn].second = q2;
    }

    auto mode = paint.getBlendMode();

    // If we have a shader, then we blend each pixel individually
    GShader *shader = paint.getShader();
    if (shader != nullptr) {
        if (!shader->setContext(transformations.top())) return;

        if (shader->isOpaque()) {
            for (int y = mn; y < mx; y++) {
                int left = row_bounds[y - mn].first;
                int right = row_bounds[y - mn].second;
                int len = right - left + 1;

                GPixel row[len];
                shader->shadeRow(left, y, len, row);
                BlitRow<true>::blend255[(int) mode](left, right, y, fDevice, row);
            }
        } else {
            for (int y = mn; y < mx; y++) {
                int left = row_bounds[y - mn].first;
                int right = row_bounds[y - mn].second;
                int len = right - left + 1;

                GPixel row[len];
                shader->shadeRow(left, y, len, row);
                BlitRow<true>::normal_blend[(int) mode](left, right, y, fDevice, row);
            }
        }

        return;
    }

    GPixel src[1] = {gutils::pixelizeFloatColor(paint.getColor())};
    if (GPixel_GetA(*src) == 255) {
        for (int y = mn; y < mx; y++)
            BlitRow<false>::blend255[(int) mode](row_bounds[y - mn].first, row_bounds[y - mn].second, y, fDevice, src);
    } else if (GPixel_GetA(*src) == 0) {
        for (int y = mn; y < mx; y++)
            BlitRow<false>::blend0[(int) mode](row_bounds[y - mn].first, row_bounds[y - mn].second, y, fDevice, src);
    } else {
        for (int y = mn; y < mx; y++)
            BlitRow<false>::normal_blend[(int) mode](row_bounds[y - mn].first, row_bounds[y - mn].second, y, fDevice,
                                                     src);
    }
}

void createQuad(std::vector<std::pair<GPoint, GPoint>> &edges, float tolerance, GPoint *points) {
    GPoint error_vec = (points[0] - 2.0f * points[1] + points[2]) * 0.25f;
    int num_segments = (int) ceil(sqrt(sqrt(error_vec.x * error_vec.x + error_vec.y * error_vec.y) / tolerance));
    float inv = 1.0f / (float) num_segments;
    GPoint prev_point = bezier::eval_quad(0.0f, points);
    float cur_t = inv;

    for (int segment = 0; segment < num_segments; segment++) {
        GPoint cur = bezier::eval_quad(cur_t, points);
        edges.emplace_back(prev_point, cur);
        cur_t += inv;
        prev_point = cur;
    }
}

void createCubic(std::vector<std::pair<GPoint, GPoint>> &edges, float tolerance, GPoint *points) {
    GPoint error_vec0 = points[0] - 2 * points[1] + points[2];
    GPoint error_vec1 = points[1] - 2 * points[2] + points[3];

    GPoint res{std::max(abs(error_vec0.x), abs(error_vec1.x)),
               std::max(abs(error_vec0.y), abs(error_vec1.y))};

    int num_segments = (int) ceil(sqrt((3 * sqrt(res.x * res.x + res.y * res.y)) / (4 * tolerance)));

    float inv = 1.0f / (float) num_segments;
    GPoint prev_point = bezier::eval_cubic(0.0f, points);
    float cur_t = inv;

    for (int segment = 0; segment < num_segments; segment++) {
        GPoint cur = bezier::eval_cubic(cur_t, points);
        edges.emplace_back(prev_point, cur);
        cur_t += inv;
        prev_point = cur;
    }
}

void GCanvas::drawPath(const GPath &path, const GPaint &paint) {
    GPath new_path = path;
    new_path.transform(transformations.top());

    GPoint points[GPath::kMaxNextPoints];
    GPath::Edger edger(new_path);

    std::vector<std::pair<GPoint, GPoint>> init_edges;
    init_edges.reserve(path.countPoints());

    std::vector<Edge> clipped;

    while (const auto verb = edger.next(points)) {
        switch (verb.value()) {
            case GPath::kLine:
                init_edges.emplace_back(points[0], points[1]);
                break;
            case GPath::kQuad:
                createQuad(init_edges, 0.25f, points);
                break;
            case GPath::kCubic:
                createCubic(init_edges, 0.25f, points);
                break;
            case GPath::kMove:
                break;
        }
    }

    clip(init_edges, clipped, fDevice.height(), fDevice.width());

    if ((int) clipped.size() < 2) return;

    std::sort(clipped.begin(), clipped.end(), [](const Edge &e1, const Edge &e2) {
        return e1.top < e2.top;
    });

    GShader *shader = paint.getShader();

    if (shader != nullptr) {
        if (!shader->setContext(transformations.top())) return;
        if (shader->isOpaque())
            drawPath<true, 255>(clipped, paint);
        else
            drawPath<true, -1>(clipped, paint);

        return;
    }

    GPixel src[1] = {gutils::pixelizeFloatColor(paint.getColor())};
    switch (GPixel_GetA(*src)) {
        case 255:
            drawPath<false, 255>(clipped, paint);
            break;
        case 0:
            drawPath<false, 0>(clipped, paint);
        default:
            drawPath<false, -1>(clipped, paint);
    }
}

void GCanvas::drawMesh(const GPoint *verts, const GColor *colors, const GPoint *texs, int count, const int *indices,
                        const GPaint &paint) {
    if (colors != nullptr && texs == nullptr) {
        for (int i = 0, n = 0; i < count; i++, n += 3) {
            GPoint draw_verts[3] = {verts[indices[n]], verts[indices[n + 1]], verts[indices[n + 2]]};
            GColor draw_colors[3] = {colors[indices[n]], colors[indices[n + 1]], colors[indices[n + 2]]};

            auto shader = GCreateTriangleGradient(draw_verts, draw_colors);
            GPaint my_paint{shader.get()};

            drawConvexPolygon(draw_verts, 3, my_paint);
        }
    }

    if (texs != nullptr) {
        for (int i = 0, n = 0; i < count; i++, n += 3) {
            GPoint draw_verts[3] = {verts[indices[n]], verts[indices[n + 1]], verts[indices[n + 2]]};
            GPoint draw_texs[3] = {texs[indices[n]], texs[indices[n + 1]], texs[indices[n + 2]]};

            GMatrix draw_mapper = gutils::compute_triangle_basis(draw_verts);
            GMatrix payload_mapper = gutils::compute_triangle_basis(draw_texs);

            auto proxy_shader = GCreateProxyShader(*paint.getShader(), draw_mapper * payload_mapper.invert().value());
            GPaint my_paint{};

            if (colors != nullptr) {
                GColor draw_colors[3] = {colors[indices[n]], colors[indices[n + 1]], colors[indices[n + 2]]};

                auto gradient_shader = GCreateTriangleGradient(draw_verts, draw_colors);
                auto compose_shader = GCreateTriangleCompose(*gradient_shader, *proxy_shader);
                my_paint.setShader(compose_shader.get());
                drawConvexPolygon(draw_verts, 3, my_paint);
            } else {
                my_paint.setShader(proxy_shader.get());
                drawConvexPolygon(draw_verts, 3, my_paint);
            }
        }
    }
}

template<typename T>
T interpolate(float s, float t, T *payload) {
    return payload[0] * (1 - s) * (1 - t) +
           payload[1] * s * (1 - t) +
           payload[2] * (1 - s) * t +
           payload[3] * s * t;
}

void GCanvas::drawQuad(const GPoint *verts, const GColor *colors, const GPoint *texs, int level, const GPaint &paint) {
    int point_cnt = level + 2;

    std::vector<GPoint> draw_points;
    draw_points.reserve(std::min(4, (point_cnt + 1) * (point_cnt + 1)));

    std::vector<GColor> draw_colors;
    draw_colors.reserve(std::min(4, (point_cnt + 1) * (point_cnt + 1)));

    std::vector<GPoint> draw_texs;
    draw_texs.reserve(std::min(4, (point_cnt + 1) * (point_cnt + 1)));

    std::vector<int> indices;
    indices.reserve(std::min(12, (point_cnt + 1) * (point_cnt) * 3));

    float step_size = 1.0f / (float) (level + 1);

    float t = 0;
    for (int i = 0; i < point_cnt; i++) {
        float s = 0;
        for (int j = 0; j < point_cnt; j++) {
            GPoint verts_payload[4] = {verts[0], verts[1], verts[3], verts[2]};
            draw_points.emplace_back(interpolate<GPoint>(s, t, verts_payload));

            if (texs != nullptr) {
                GPoint texs_payload[4] = {texs[0], texs[1], texs[3], texs[2]};
                draw_texs.emplace_back(interpolate<GPoint>(s, t, texs_payload));
            }

            if (colors != nullptr) {
                GColor colors_payload[4] = {colors[0], colors[1], colors[3], colors[2]};
                draw_colors.emplace_back(interpolate<GColor>(s, t, colors_payload));
            }

            s += step_size;
        }

        t += step_size;
    }

    for (int i = 0; i < point_cnt - 1; i++) {
        for (int j = 0; j < point_cnt; j++) {
            int cur_idx = i * point_cnt + j;

            if (j < point_cnt - 1) {
                indices.push_back(cur_idx);
                indices.push_back(cur_idx + 1);
                indices.push_back(cur_idx + point_cnt);
            }

            if (j >= 1) {
                indices.push_back(cur_idx);
                indices.push_back(cur_idx + point_cnt - 1);
                indices.push_back(cur_idx + point_cnt);
            }

        }
    }

    drawMesh(draw_points.data(), (colors == nullptr ? nullptr : draw_colors.data()),
             (texs == nullptr ? nullptr : draw_texs.data()), (int) indices.size() / 3, indices.data(), paint);
}

template<bool has_shader, int alpha>
void GCanvas::drawPath(std::vector<Edge> &clipped, const GPaint &paint) {
    int top_y = clipped.front().top;
    int bottom_y = clipped.back().bottom;
    for (const auto &edge: clipped) {
        top_y = std::min(top_y, edge.top);
        bottom_y = std::max(bottom_y, edge.bottom);
    }

    int num_edges = (int) clipped.size();

    std::vector<int> next_edge(num_edges);
    std::iota(next_edge.begin(), next_edge.end(), 1);

    std::vector<std::pair<int, int>> x_vals;
    x_vals.reserve(num_edges);

    GShader *shader = paint.getShader();
    int mode = (int) paint.getBlendMode();
    GPixel src[1] = {gutils::pixelizeFloatColor(paint.getColor())};

    int start_idx = 0;

    for (int y = top_y; y < bottom_y; y++) {
        x_vals.clear();

        int prev_idx = start_idx, cur_idx = start_idx;

        while (cur_idx < num_edges) {
            if (clipped[cur_idx].bottom <= y) {
                if (cur_idx == start_idx) {
                    start_idx = next_edge[cur_idx];
                    prev_idx = start_idx;
                } else {
                    next_edge[prev_idx] = next_edge[cur_idx];
                }

                cur_idx = next_edge[cur_idx];
                continue;
            }

            if (!(cur_idx < num_edges && gutils::is_inside(y, clipped[cur_idx].top, clipped[cur_idx].bottom))) break;
            x_vals.emplace_back(clipped[cur_idx].query_x_round((float) y + 0.5f), clipped[cur_idx].winding);

            if (gutils::is_inside(y + 1, clipped[cur_idx].top, clipped[cur_idx].bottom)) {
                prev_idx = cur_idx;
            } else {
                if (cur_idx == start_idx) {
                    start_idx = next_edge[cur_idx];
                    prev_idx = start_idx;
                } else {
                    next_edge[prev_idx] = next_edge[cur_idx];
                }
            }

            cur_idx = next_edge[cur_idx];
        }

        std::sort(x_vals.begin(), x_vals.end());

        int cur_winding = 0, l = 0, r = 0;

        for (const auto [x, orientation]: x_vals) {
            if (cur_winding == 0)
                l = x;

            cur_winding += orientation;

            if (cur_winding == 0) {
                r = x;

                if (shader != nullptr) {
                    int len = r - l + 1;
                    GPixel row[len];
                    shader->shadeRow(l, y, len, row);

                    if (alpha == 255)
                        BlitRow<true>::blend255[mode](l, r, y, fDevice, row);
                    else
                        BlitRow<true>::normal_blend[mode](l, r, y, fDevice, row);
                } else {
                    if (alpha == 255) {
                        BlitRow<false>::blend255[(int) mode](l, r, y, fDevice, src);
                    } else if (alpha == 0) {
                        BlitRow<false>::blend0[(int) mode](l, r, y, fDevice, src);
                    } else {
                        BlitRow<true>::normal_blend[(int) mode](l, r, y, fDevice, src);
                    }
                }
            }
        }
    }
}


std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap &device) {
    return std::unique_ptr<GCanvas>(new GCanvas(device));
}

std::string GDrawSomething(GCanvas *canvas, GISize dim) {
//    GBitmap bm;
//    bm.readFromFile("apps/spock.png");
//    auto shader = GCreateBitmapShader(bm, GMatrix::Translate(30, -100) * GMatrix::Rotate(20));
//
//    GPaint paint;
//    paint.setShader(shader.get());
//
//    GPoint pts[] = {{0,   0},
//                    {0,   256},
//                    {256, 256},
//                    {256, 0}};
//
//    canvas->drawConvexPolygon(pts, 4, paint);
//
//    GBitmap bm2;
//    bm2.readFromFile("apps/wheel.png");
//    auto shader2 = GCreateBitmapShader(bm2, GMatrix::Translate(100, 100) * GMatrix::Scale(2, 2));
//    paint.setShader(shader2.get());
//    paint.setBlendMode(GBlendMode::kSrcOver);
//
//    canvas->drawConvexPolygon(pts, 4, paint);
//    GPoint verts[4] = {{10,  50},
//                       {200, 50},
//                       {200, 200},
//                       {10,  200}};
//
//    GColor c[1] = {{0, 0, 1, 1}};
//
//    canvas->drawQuad(verts, c, nullptr, 12, GPaint());

    return "My Drawing";
}