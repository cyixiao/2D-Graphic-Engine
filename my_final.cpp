#include "include/GFinal.h"
#include "include/GShader.h"
#include "include/GBitmap.h"
#include "my_bilerpShader.h"
#include "my_radialGradient.h"

class MyFinal : public GFinal {

    std::unique_ptr<GShader> createBilerpShader(const GBitmap& bitmap,
                                                const GMatrix& localMatrix) override {
        return std::unique_ptr<GShader>(new BilerpShader(bitmap, localMatrix));
    }

    void addLine(GPath* path, GPoint p0, GPoint p1, float width, CapType type) override {
        float radius = width / 2;
        float angle = atan(-1 / ((p1.y - p0.y) / (p1.x - p0.x)));
        float dx = radius * cos(angle);
        float dy = radius * sin(angle);
        switch(type){
            case kButt:
                path -> moveTo(GPoint{p0.x + dx, p0.y + dy});
                path -> lineTo(GPoint{p0.x - dx, p0.y - dy});
                path -> lineTo(GPoint{p1.x - dx, p1.y - dy});
                path -> lineTo(GPoint{p1.x + dx, p1.y + dy});
                break;
            case kSquare:
                path -> moveTo(GPoint{p0.x + dx + dy, p0.y + dx + dy});
                path -> lineTo(GPoint{p0.x - dx + dy, p0.y - dy - dx});
                path -> lineTo(GPoint{p1.x - dx - dy, p1.y - dx - dy});
                path -> lineTo(GPoint{p1.x + dx - dy, p1.y + dy - dx});
                break;
            case kRound:
                path -> addCircle(p0, radius, GPath::Direction::kCCW_Direction);
                path -> moveTo(GPoint{p0.x + dx, p0.y + dy});
                path -> lineTo(GPoint{p0.x - dx, p0.y - dy});
                path -> lineTo(GPoint{p1.x - dx, p1.y - dy});
                path -> lineTo(GPoint{p1.x + dx, p1.y + dy});
                path -> addCircle(p1, radius, GPath::Direction::kCCW_Direction);
                break;
            default: return;
        }
    }
    std::unique_ptr<GShader> createRadialGradient(GPoint center, float radius, 
                                                const GColor colors[], int count, 
                                                GShader::TileMode tileMode) override {
        return std::unique_ptr<GShader>(new RadialGradientShader(center, radius, colors, count, tileMode));
    }

    GPoint evaluateBezier(const GPoint p0, const GPoint p1, const GPoint p2, float t) {
        float invT = 1 - t;
        return invT * invT * p0 + 2 * invT * t * p1 + t * t * p2;
    }

    GPoint lerpPoints(const GPoint p0, const GPoint p1, float t) {
        return p0 + t * (p1 - p0);
    }

    GPoint bilinearLerp(const GPoint c0, const GPoint c1, const GPoint c2, const GPoint c3, float u, float v) {
        GPoint top = lerpPoints(c0, c1, u);
        GPoint bottom = lerpPoints(c3, c2, u);
        return lerpPoints(top, bottom, v);
    }

    void drawQuadraticCoons(GCanvas* canvas, const GPoint pts[8], const GPoint tex[4], int level, const GPaint& paint) override{
        int pointsNum = level + 1;
        GPoint quadVerts[4];
        GPoint quadTexs[4];
        for (int r = 0; r < pointsNum; ++r) {
            for (int c = 0; c < pointsNum; ++c) {
                for (int i = 0; i < 2; ++i) {
                    for (int j = 0; j < 2; ++j) {
                        float u = (float)(c + j) / pointsNum;
                        float v = (float)(r + i) / pointsNum;
                        int index = i * 2 + j;
                        // evaluate the top and bottom Bezier curves at u
                        GPoint top = evaluateBezier(pts[0], pts[1], pts[2], u);
                        GPoint bottom = evaluateBezier(pts[6], pts[5], pts[4], u);
                        // evaluate the left and right Bezier curves at v
                        GPoint left = evaluateBezier(pts[0], pts[7], pts[6], v);
                        GPoint right = evaluateBezier(pts[2], pts[3], pts[4], v);
                        // linearly lerp between top and bottom, and left and right
                        GPoint TB = lerpPoints(top, bottom, v);
                        GPoint LR = lerpPoints(left, right, u);
                        // bilinear lerp of the corner points
                        GPoint corners = bilinearLerp(pts[0], pts[2], pts[4], pts[6], u, v);
                        // coons formula
                        quadVerts[index] = TB + LR - corners;
                        // lerp of tex points
                        quadTexs[index] = bilinearLerp(tex[0], tex[1], tex[2], tex[3], u, v);
                    }
                }
                // The local indices
                int localIndices[6] = {0, 2, 3, 0, 1, 3};
                // draw the quad using drawMesh
                canvas->drawMesh(quadVerts, nullptr, quadTexs, 2, localIndices, paint);
            }
        }
    }
};

std::unique_ptr<GFinal> GCreateFinal() {
    return std::unique_ptr<GFinal>(new MyFinal());
}