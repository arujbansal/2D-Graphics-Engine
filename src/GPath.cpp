/*
 *  Copyright 2024 Aruj Bansal
 */

#include "../include/GPath.h"
#include "../include/GBezier.h"

GPath::GPath() {}

GPath::~GPath() {}

GPath &GPath::operator=(const GPath &src) {
    if (this != &src) {
        fPts = src.fPts;
        fVbs = src.fVbs;
    }
    return *this;
}

void GPath::reset() {
    fPts.clear();
    fVbs.clear();
}

void GPath::dump() const {
    Iter iter(*this);
    GPoint pts[GPath::kMaxNextPoints];
    while (auto v = iter.next(pts)) {
        switch (*v) {
            case kMove:
                printf("M %g %g\n", pts[0].x, pts[0].y);
                break;
            case kLine:
                printf("L %g %g\n", pts[1].x, pts[1].y);
                break;
            case kQuad:
                printf("Q %g %g  %g %g\n", pts[1].x, pts[1].y, pts[2].x, pts[2].y);
                break;
            case kCubic:
                printf("C %g %g  %g %g  %g %g\n",
                       pts[1].x, pts[1].y,
                       pts[2].x, pts[2].y,
                       pts[3].x, pts[3].y);
                break;
        }
    }
}

void GPath::quadTo(GPoint p1, GPoint p2) {
    assert(fVbs.size() > 0);
    fPts.push_back(p1);
    fPts.push_back(p2);
    fVbs.push_back(kQuad);
}

void GPath::cubicTo(GPoint p1, GPoint p2, GPoint p3) {
    assert(fVbs.size() > 0);
    fPts.push_back(p1);
    fPts.push_back(p2);
    fPts.push_back(p3);
    fVbs.push_back(kCubic);
}

/////////////////////////////////////////////////////////////////

GPath::Iter::Iter(const GPath &path) {
    fCurrPt = path.fPts.data();
    fCurrVb = path.fVbs.data();
    fStopVb = fCurrVb + path.fVbs.size();
}

std::optional<GPath::Verb> GPath::Iter::next(GPoint pts[]) {
    assert(fCurrVb <= fStopVb);
    if (fCurrVb == fStopVb) {
        return {};
    }
    Verb v = *fCurrVb++;
    switch (v) {
        case kMove:
            pts[0] = *fCurrPt++;
            break;
        case kLine:
            pts[0] = fCurrPt[-1];
            pts[1] = *fCurrPt++;
            break;
        case kQuad:
            pts[0] = fCurrPt[-1];
            pts[1] = *fCurrPt++;
            pts[2] = *fCurrPt++;
            break;
        case kCubic:
            pts[0] = fCurrPt[-1];
            pts[1] = *fCurrPt++;
            pts[2] = *fCurrPt++;
            pts[3] = *fCurrPt++;
            break;
    }
    return v;
}

constexpr int kDoneVerb = -1;

GPath::Edger::Edger(const GPath &path) {
    fPrevMove = nullptr;
    fCurrPt = path.fPts.data();
    fCurrVb = path.fVbs.data();
    fStopVb = fCurrVb + path.fVbs.size();
    fPrevVerb = kDoneVerb;
}

std::optional<GPath::Verb> GPath::Edger::next(GPoint pts[]) {
    assert(fCurrVb <= fStopVb);
    bool do_return = false;
    while (fCurrVb < fStopVb) {
        switch (*fCurrVb++) {
            case kMove:
                if (fPrevVerb == kLine) {
                    pts[0] = fCurrPt[-1];
                    pts[1] = *fPrevMove;
                    do_return = true;
                }
                fPrevMove = fCurrPt++;
                fPrevVerb = kMove;
                break;
            case kLine:
                pts[0] = fCurrPt[-1];
                pts[1] = *fCurrPt++;
                fPrevVerb = kLine;
                return kLine;
            case kQuad:
                pts[0] = fCurrPt[-1];
                pts[1] = *fCurrPt++;
                pts[2] = *fCurrPt++;
                fPrevVerb = kQuad;
                return kQuad;
            case kCubic:
                pts[0] = fCurrPt[-1];
                pts[1] = *fCurrPt++;
                pts[2] = *fCurrPt++;
                pts[3] = *fCurrPt++;
                fPrevVerb = kCubic;
                return kCubic;
        }
        if (do_return) {
            return kLine;
        }
    }
    if (fPrevVerb >= kLine && fPrevVerb <= kCubic) {
        pts[0] = fCurrPt[-1];
        pts[1] = *fPrevMove;
        fPrevVerb = kDoneVerb;
        return kLine;
    } else {
        return {};
    }
}


void GPath::addRect(const GRect &rect, Direction dir) {
    moveTo(rect.left, rect.top);

    if (dir == kCW_Direction) {
        lineTo(rect.right, rect.top);
        lineTo(rect.right, rect.bottom);
        lineTo(rect.left, rect.bottom);
        return;
    }

    // Counter-clockwise
    lineTo(rect.left, rect.bottom);
    lineTo(rect.right, rect.bottom);
    lineTo(rect.right, rect.top);
}

void GPath::addPolygon(const GPoint *pts, int count) {
    moveTo(pts[0]);

    for (int i = 1; i < count; i++)
        lineTo(pts[i]);
}

void GPath::addCircle(GPoint center, float radius, GPath::Direction direction) {
    GMatrix transformer = GMatrix::Translate(center.x, center.y) * GMatrix::Scale(radius, radius);

    moveTo(transformer * GPoint{0.0f, 1.0f});

    if (direction == kCCW_Direction) {
        GPoint points[] = {0.551915f, 1.0f,
                           1.0f, 0.551915f,
                           1.0f, 0.0f,
                           1.0f, -0.551915f,
                           0.551915f, -1.0f,
                           0.0f, -1.0f,
                           -0.551915f, -1.0f,
                           -1.0f, -0.551915f,
                           -1.0f, -0.0f,
                           -1.0f, 0.551915f,
                           -0.551915f, 1.0f,
                           0.0f, 1.0f};

        transformer.mapPoints(points, points, 12);

        cubicTo(points[0], points[1], points[2]);
        cubicTo(points[3], points[4], points[5]);
        cubicTo(points[6], points[7], points[8]);
        cubicTo(points[9], points[10], points[11]);
    } else if (direction == kCW_Direction) {
        GPoint points[] = {-0.551915f, 1.0f,
                           -1.0f, 0.551915f,
                           -1.0f, 0.0f,
                           -1.0f, -0.551915f,
                           -0.551915f, -1.0f,
                           0.0f, -1.0f,
                           0.551915f, -1.0f,
                           1.0f, -0.551915f,
                           1.0f, 0.0f,
                           1.0f, 0.551915f,
                           0.551915f, 1.0f,
                           0.0f, 1.0f};

        transformer.mapPoints(points, points, 12);

        cubicTo(points[0], points[1], points[2]);
        cubicTo(points[3], points[4], points[5]);
        cubicTo(points[6], points[7], points[8]);
        cubicTo(points[9], points[10], points[11]);
    }
}

GRect GPath::bounds() const {
    int num_points = countPoints();

    if (num_points == 0)
        return GRect::LTRB(0.0f, 0.0f, 0.0f, 0.0f);

    std::pair<float, float> x_bounds = std::make_pair(10000, -10000); // {left, right}
    std::pair<float, float> y_bounds = std::make_pair(10000, -10000); // {top, bottom}

    Edger edger(*this);
    GPoint points[GPath::kMaxNextPoints];

    while (const auto verb = edger.next(points))
        if (verb.value() == kLine) {
            x_bounds.first = std::min(x_bounds.first, points[0].x);
            x_bounds.second = std::max(x_bounds.second, points[0].x);
            x_bounds.first = std::min(x_bounds.first, points[1].x);
            x_bounds.second = std::max(x_bounds.second, points[1].x);

            y_bounds.first = std::min(y_bounds.first, points[0].y);
            y_bounds.second = std::max(y_bounds.second, points[0].y);
            y_bounds.first = std::min(y_bounds.first, points[1].y);
            y_bounds.second = std::max(y_bounds.second, points[1].y);
        } else if (verb.value() == kQuad) {
            float dx = bezier::derivative_zero_quad(points[0].x, points[1].x, points[2].x);
            float dy = bezier::derivative_zero_quad(points[0].y, points[1].y, points[2].y);

            if (dx != -1.0) {
                GPoint x_eval = bezier::eval_quad(dx, points);
                x_bounds.first = std::min(x_bounds.first, x_eval.x);
                x_bounds.second = std::max(x_bounds.second, x_eval.x);
            }

            if (dy != -1.0) {
                GPoint y_eval = bezier::eval_quad(dy, points);
                y_bounds.first = std::min(y_bounds.first, y_eval.y);
                y_bounds.second = std::max(y_bounds.second, y_eval.y);
            }

            x_bounds.first = std::min(x_bounds.first, points[0].x);
            x_bounds.second = std::max(x_bounds.second, points[0].x);
            x_bounds.first = std::min(x_bounds.first, points[2].x);
            x_bounds.second = std::max(x_bounds.second, points[2].x);

            y_bounds.first = std::min(y_bounds.first, points[0].y);
            y_bounds.second = std::max(y_bounds.second, points[0].y);
            y_bounds.first = std::min(y_bounds.first, points[2].y);
            y_bounds.second = std::max(y_bounds.second, points[2].y);

        } else if (verb.value() == kCubic) {
            std::pair<float, float> dx = bezier::derivative_zero_cubic(points[0].x, points[1].x, points[2].x,
                                                                       points[3].x);
            std::pair<float, float> dy = bezier::derivative_zero_cubic(points[0].y, points[1].y, points[2].y,
                                                                       points[3].y);

            if (dx.first + 1.0f > 0.000000001) {
                GPoint x_eval1 = bezier::eval_cubic(dx.first, points);
                x_bounds.first = std::min(x_bounds.first, x_eval1.x);
                x_bounds.second = std::max(x_bounds.second, x_eval1.x);

                GPoint x_eval2 = bezier::eval_cubic(dx.second, points);
                x_bounds.first = std::min(x_bounds.first, x_eval2.x);
                x_bounds.second = std::max(x_bounds.second, x_eval2.x);

//                printf("x_eval: %f %f\n", x_eval1.x, x_eval2.x);
            }

            if (dy.first + 1.0f > 0.000000001f) {
                GPoint y_eval1 = bezier::eval_cubic(dy.first, points);
                y_bounds.first = std::min(y_bounds.first, y_eval1.y);
                y_bounds.second = std::max(y_bounds.second, y_eval1.y);

                GPoint y_eval2 = bezier::eval_cubic(dy.second, points);
                y_bounds.first = std::min(y_bounds.first, y_eval2.y);
                y_bounds.second = std::max(y_bounds.second, y_eval2.y);

//                printf("y_eval: %f %f\n", y_eval1.y, y_eval2.y);
            }


            x_bounds.first = std::min(x_bounds.first, points[0].x);
            x_bounds.first = std::min(x_bounds.first, points[2].x);
            x_bounds.second = std::max(x_bounds.second, points[0].x);
            x_bounds.second = std::max(x_bounds.second, points[2].x);

//            printf("x_control_eval: %f %f\n", points[0].x, points[3].x);

            y_bounds.first = std::min(y_bounds.first, points[0].y);
            y_bounds.first = std::min(y_bounds.first, points[2].y);
            y_bounds.second = std::max(y_bounds.second, points[0].y);
            y_bounds.second = std::max(y_bounds.second, points[2].y);
        }

    auto computed_bounds = GRect::LTRB(x_bounds.first, y_bounds.first, x_bounds.second, y_bounds.second);
//    printf("%f %f %f %f", bou.left, bou.top, bou.right, bou.bottom);
    return computed_bounds;
}

void GPath::transform(const GMatrix &transformer) {
    int num_points = countPoints();

    for (int i = 0; i < num_points; i++)
        fPts[i] = transformer * fPts[i];
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    bezier::chop_quad_at(src, dst, t);
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    bezier::chop_cubic_at(src, dst, t);
}