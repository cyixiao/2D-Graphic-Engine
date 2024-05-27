#include "include/GPath.h"

void GPath::addRect(const GRect& rect, Direction direction){
    moveTo(rect.left, rect.top);
    if(direction == Direction::kCW_Direction){
        lineTo({rect.right, rect.top});
        lineTo({rect.right, rect.bottom});
        lineTo({rect.left, rect.bottom});
    } else {
        lineTo({rect.left, rect.bottom});
        lineTo({rect.right, rect.bottom});
        lineTo({rect.right, rect.top});
    }
}

void GPath::addPolygon(const GPoint pts[], int count){
    moveTo(pts[0]);
    for(int i = 1; i < count; ++i){
        lineTo(pts[i]);
    }
}

void GPath::addCircle(GPoint center, float radius, Direction direct){
    float k = 0.552285f;
    GMatrix ctm = GMatrix::Translate(center.x, center.y) * GMatrix::Scale(radius, radius);
    if (direct == Direction::kCW_Direction) {
        moveTo(ctm * GPoint{0, 1});
        cubicTo(ctm * GPoint{-k, 1}, ctm * GPoint{-1, k}, ctm * GPoint{-1, 0});
        cubicTo(ctm * GPoint{-1, -k}, ctm * GPoint{-k, -1}, ctm * GPoint{0, -1});
        cubicTo(ctm * GPoint{k, -1}, ctm * GPoint{1, -k}, ctm * GPoint{1, 0});
        cubicTo(ctm * GPoint{1, k}, ctm * GPoint{k, 1}, ctm * GPoint{0, 1});
    } else {
        moveTo(ctm * GPoint{0, 1});
        cubicTo(ctm * GPoint{k, 1}, ctm * GPoint{1, k}, ctm * GPoint{1, 0});
        cubicTo(ctm * GPoint{1, -k}, ctm * GPoint{k, -1}, ctm * GPoint{0, -1});
        cubicTo(ctm * GPoint{-k, -1}, ctm * GPoint{-1, -k}, ctm * GPoint{-1, 0});
        cubicTo(ctm * GPoint{-1, k}, ctm * GPoint{-k, 1}, ctm * GPoint{0, 1});
    }
}

void computeQuadExtre(const GPoint& p0, const GPoint& p1, const GPoint& p2, std::vector<float>& ts){
    float t;
    t = (p1.x - p0.x) / ((p1.x - p0.x) - (p2.x - p1.x));
    if(t >= 0 && t <= 1){
        ts.push_back(t);
    }
    t = (p1.y - p0.y) / ((p1.y - p0.y) - (p2.y - p1.y));
    if(t >= 0 && t <= 1){
        ts.push_back(t);
    }
}

void computeCubicExtre(const GPoint& p0, const GPoint& p1, const GPoint& p2, const GPoint& p3, std::vector<float>& ts){
    float t;

    float ax = 3.0f * (3.0f * p1.x - 3.0f * p2.x + p3.x - p0.x);
    float bx = 6.0f * (p0.x - 2.0f * p1.x + p2.x);
    float cx = 3.0f * (p1.x - p0.x);
    float dx = bx * bx - 4.0f * ax * cx;

    if(ax == 0){
        if(bx != 0){
            float tx = -1 * cx / bx;
            ts.push_back(tx);
        }
    }

    if(dx > 0 && ax != 0){
        t = (-1 * bx + sqrt(dx)) / (2 * ax);
        if(t >= 0 && t <= 1){
            ts.push_back(t);
        }
        t = (-1 * bx - sqrt(dx)) / (2 * ax);
        if(t >= 0 && t <= 1){
            ts.push_back(t);
        }
    }
    float ay = 3.0f * (3.0f * p1.y - 3.0f * p2.y + p3.y - p0.y);
    float by = 6.0f * (p0.y - 2.0f * p1.y + p2.y);
    float cy = 3.0f * (p1.y - p0.y);
    float dy = by * by - 4.0f * ay * cy;

    if(ay == 0){
        if(by != 0){
            float ty = -1 * cy / by;
            ts.push_back(ty);
        }
    }
    if(dy > 0 && ay != 0){
        t = (-1 * by + sqrt(dy)) / (2 * ay);

        if(t >= 0 && t <= 1){
            ts.push_back(t);
        }

        t = (-1 * by - sqrt(dy)) / (2 * ay);

        if(t >= 0 && t <= 1){
            ts.push_back(t);
        }
    }
}

GRect GPath::bounds() const{
    if(fPts.size() < 1){
        return GRect::LTRB(0, 0, 0, 0);
    }
    float left = fPts[0].x;
    float top = fPts[0].y;
    float right = fPts[0].x;
    float bottom = fPts[0].y;
    GPoint pts[4];
    std::vector<float> ts;
    Edger edger(*this);
    Verb v = edger.next(pts);
    while (v != kDone) {
        if(v == GPath::kLine){
            left = std::min(left, pts[1].x);
            top = std::min(top, pts[1].y);
            right = std::max(right, pts[1].x);
            bottom = std::max(bottom, pts[1].y);
        } else if(v == GPath::kQuad){
            left = std::min(left, pts[2].x);
            top = std::min(top, pts[2].y);
            right = std::max(right, pts[2].x);
            bottom = std::max(bottom, pts[2].y);
            computeQuadExtre(pts[0], pts[1], pts[2], ts);
            while(!ts.empty()){
                float t = ts.back();
                ts.pop_back();

                GPoint dst[5];
                ChopQuadAt(pts, dst, t);
                left = std::min(left, dst[2].x);
                top = std::min(top, dst[2].y);
                right = std::max(right, dst[2].x);
                bottom = std::max(bottom, dst[2].y);
            }
            ts.clear();
        } else if(v == GPath::kCubic){
            left = std::min(left, pts[3].x);
            top = std::min(top, pts[3].y);
            right = std::max(right, pts[3].x);
            bottom = std::max(bottom, pts[3].y);
            computeCubicExtre(pts[0], pts[1], pts[2], pts[3], ts);
            while(!ts.empty()){
                float t = ts.back();
                ts.pop_back();
                GPoint dst[7];
                ChopCubicAt(pts, dst, t);
                left = std::min(left, dst[3].x);
                top = std::min(top, dst[3].y);
                right = std::max(right, dst[3].x);
                bottom = std::max(bottom, dst[3].y);
            }
            ts.clear();
        }
        v = edger.next(pts);
    }
    return GRect::LTRB(left, top, right, bottom);
}

void GPath::transform(const GMatrix& ctm){
    for(int i = 0; i < fPts.size(); ++i){
        fPts[i] = ctm * fPts[i];
    }
}

inline GPoint bezier(const GPoint& a, const GPoint& b, float t){
    return (1 - t) * a + t * b;
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t){
    // P0
    dst[0] = src[0];
    // L0: on P0, P1
    dst[1] = bezier(src[0], src[1], t);
    // L1: on P1, P2
    dst[3] = bezier(src[1], src[2], t);
    // Q0: on L0, L1
    dst[2] = bezier(dst[1], dst[3], t);
    // P3
    dst[4] = src[2];
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t){
    // P0
    dst[0] = src[0];
    // L0: on P0, P1
    dst[1] = bezier(src[0], src[1], t);
    // L1: on P1, P2
    GPoint L1 = bezier(src[1], src[2], t);
    // Q0: on L0, L1
    dst[2] = bezier(dst[1], L1, t);
    // L2: on P2, P3
    dst[5] = bezier(src[2], src[3], t);
    // Q1: on L1, L2
    dst[4] = bezier(L1, dst[5], t);
    // C0: on Q0, Q1
    dst[3] = bezier(dst[2], dst[4], t);
    // P3
    dst[6] = src[3];
}