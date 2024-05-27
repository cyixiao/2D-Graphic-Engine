#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/GPixel.h"
#include "include/GPoint.h"
#include "include/GPaint.h"
#include "include/GPath.h"
#include "include/GMath.h"
#include "include/GMatrix.h"
#include "include/GShader.h"
#include "my_blendMode.h"
#include "my_edge.h"
#include "my_triGradient.h"
#include "my_proxyShader.h"
#include "my_composeShader.h"
#include "my_gradient.h"
#include <stack>

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device) {
        ctmStack.push(GMatrix());
    }

    void clear(const GColor& color) override {
        GPixel pixel = colorToPixel(color);
        for(int i = 0; i < fDevice.height(); ++i){
            for(int j = 0; j < fDevice.width(); ++j){
                *fDevice.getAddr(j, i) = pixel;
            }
        }
    }
    
    void drawRect(const GRect& rect, const GPaint& paint) override {
        GIRect rectp = rect.round();
        rectp.left = std::max(0, rectp.left);
        rectp.top = std::max(0, rectp.top);
        rectp.right = std::max(std::min(fDevice.width(), rectp.right), 0);
        rectp.bottom = std::max(std::min(fDevice.height(), rectp.bottom), 0);
        
        if(ctmStack.top() == GMatrix() && paint.getShader() == nullptr){
            GPixel pixel = colorToPixel(paint.getColor());
            BlendingFor proc = optimizing(paint.getBlendMode(), GPixel_GetA(pixel));
            proc(fDevice, rectp, pixel);
        } else{
            GPoint points[] = {
            {rect.left, rect.top},
            {rect.left, rect.bottom},
            {rect.right, rect.bottom},
            {rect.right, rect.top}
            };
            drawConvexPolygon(points, 4, paint);
        }
    }

    void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override {
        if(count < 3){
            return;
        }
        GPoint transPoint[count];
        ctmStack.top().mapPoints(transPoint, points, count);
        std::vector<Edge> edges = createPloygon(transPoint, count, GRect::WH(fDevice.width(), fDevice.height()));
        if(edges.size() < 2){
            return;
        }
        int rayBegin = edges.back().ymin;
        int rayEnd = edges.front().ymax;
        Edge leftE = edges.back();
        edges.pop_back();
        Edge rightE = edges.back();
        edges.pop_back();
        float leftX = leftE.topx;
        float rightX = rightE.topx;

        GPixel pixel = colorToPixel(paint.getColor());
        BlendingNor procNor = optimizingNor(paint.getBlendMode(), GPixel_GetA(pixel));
        BlendingSha procSha = optimizingSha(paint.getBlendMode());

        for(int i = rayBegin; i < rayEnd; ++i){
            if(i >= leftE.ymax){
                leftE = edges.back();
                edges.pop_back();
                leftX = leftE.topx;
            } 
            if(i >= rightE.ymax){
                rightE = edges.back();
                edges.pop_back();
                rightX = rightE.topx;
            } 
            int tempLeft = GRoundToInt(leftX);
            int tempRight = GRoundToInt(rightX);
            blit(paint, pixel, procNor,procSha, i, tempLeft, tempRight);
            
            leftX += leftE.m;
            rightX += rightE.m;
        }
    }

    void drawPath(const GPath& path, const GPaint& paint) override {
        // get edges
        GRect canvas = GRect::WH(fDevice.width(), fDevice.height());
        GPath p = path;
        p.transform(ctmStack.top());
        std::vector<Edge> edges;
        GPoint pts[4];
        GPath::Edger edger(p);
        GPath::Verb verb = edger.next(pts);
        while(verb != GPath::kDone){
            if(verb == GPath::kLine){
                clip(canvas, pts[0], pts[1], edges);
            } else if(verb == GPath::kQuad){
                GPoint p0 = pts[0];
                GPoint E = (pts[0] - 2 * pts[1] + pts[2]) * 0.25;
                int num = GCeilToInt(sqrt(4 * E.length()));
                float incr = 1.0f / num;
                float t = incr;
                GPoint dstPoints[5];
                for(int i = 0; i < num; ++i){
                    GPath::ChopQuadAt(pts, dstPoints, t);
                    GPoint p1 = dstPoints[2];
                    clip(canvas, p0, p1, edges);
                    p0 = p1;
                    t += incr;
                }
                clip(canvas, p0, pts[2], edges);
            } else if(verb == GPath::kCubic){
                GPoint p0 = pts[0];
                GPoint E0 = pts[0] - pts[1] - pts[1] + pts[2];
                GPoint E1 = pts[1] - pts[2] - pts[2] + pts[3];
                GPoint E = {std::max(std::abs(E0.x), std::abs(E1.x)), 
                            std::max(std::abs(E0.y), std::abs(E1.y))}; 
                int num = GCeilToInt(sqrt(E.length() * 3));
                float incr = 1.0f / num;
                float t = incr;
                GPoint dstPoints[7];
                for(int i = 0; i < num; ++i){
                    GPath::ChopCubicAt(pts, dstPoints, t);
                    GPoint p1 = dstPoints[3];
                    clip(canvas, p0, p1, edges);
                    p0 = p1;
                    t += incr;
                }
                clip(canvas, p0, pts[3], edges);
            }
            verb = edger.next(pts);
        }
        if(edges.size() < 1){
            return;
        }
        std::sort(edges.begin(), edges.end(), comparePath);
        
        //prepare for blit
        GPixel pixel = colorToPixel(paint.getColor());
        BlendingNor procNor = optimizingNor(paint.getBlendMode(), GPixel_GetA(pixel));
        BlendingSha procSha = optimizingSha(paint.getBlendMode());

        // complex scan-converter
        int y = edges[0].ymin;
        int left;
        int right;
        while(edges.size() > 0){
            int index = 0;
            int w = 0;
            while(index < edges.size() && edges[index].isValid(y)){
                int x = edges[index].computeX(y);
                if(w == 0){
                    left = x;
                }
                w += edges[index].wind;
                if(w == 0){
                    right = x;
                    blit(paint, pixel, procNor,procSha, y, left, right);
                }
                if(!edges[index].isValid(y + 1)){
                    edges.erase(edges.begin() + index);
                } else{
                    edges[index].ex += edges[index].m;
                    ++index;
                }
            }
            ++y;
            while(index < edges.size() && edges[index].isValid(y)){
                ++index;
            }
            std::sort(edges.begin(), edges.begin() + index, compareX);
        }
    }

    inline void blit(const GPaint& paint, GPixel pixel, BlendingNor& procNor, BlendingSha& procSha, int y, int left, int right){
        left = std::max(left, 0);
        right = std::min(right,fDevice.width());
        int width = right - left;
        if(paint.getShader() != nullptr){
            if(!paint.getShader() -> setContext(ctmStack.top())){
                return;
            }
            GPixel row[width];
            paint.getShader() -> shadeRow(left, y, width, row);
            
            if(paint.getShader()->isOpaque()){
                for(int x = left; x < right; ++x){
                    *fDevice.getAddr(x, y) = row[x - left];
                }
            } else{
                procSha(fDevice, left, right, y, row);
            }
        } else{
            procNor(fDevice, left, right, y, pixel);
        }
    }

    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) override {
        int n = 0;
        for(int i = 0; i < count; ++i){
            GPoint p[3] = {verts[indices[n + 0]],
                            verts[indices[n + 1]],
                            verts[indices[n + 2]]};
            if(colors != nullptr && texs != nullptr){
                GColor c[3] = {colors[indices[n + 0]],
                                colors[indices[n + 1]],
                                colors[indices[n + 2]]};
                GPoint t[3] = {texs[indices[n + 0]],
                                texs[indices[n + 1]],
                                texs[indices[n + 2]]};
                GPaint composePaint = (new ComposeShader(new TriGradient(c, p), new ProxyShader(p, t, paint.getShader())));
                drawConvexPolygon(p, 3, composePaint);
            } else if (colors != nullptr){
                GColor c[3] = {colors[indices[n + 0]],
                                colors[indices[n + 1]],
                                colors[indices[n + 2]]};
                GPaint colorPaint(new TriGradient(c, p));
                drawConvexPolygon(p, 3, colorPaint);
            } else if (texs != nullptr){
                GPoint t[3] = {texs[indices[n + 0]],
                                texs[indices[n + 1]],
                                texs[indices[n + 2]]};
                GPaint texPaint(new ProxyShader(p, t, paint.getShader()));
                drawConvexPolygon(p, 3, texPaint);
            } else{
                drawConvexPolygon(p, 3, paint);
            }
            n += 3;
        }
    }

    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) override {
        int pointsNum = level + 1;
        GPoint meshVerts[4];
        GColor meshColors[4];
        GPoint meshTexs[4];
        int localIndices[6] = {0, 1, 2, 1, 2, 3};
        for (int r = 0; r < pointsNum; ++r) {
            for (int c = 0; c < pointsNum; ++c) {
                for (int i = 0; i < 2; ++i) {
                    for (int j = 0; j < 2; ++j) {
                        float row = (float)(r + i) / pointsNum;
                        float column = (float)(c + j) / pointsNum;
                        int index = i * 2 + j;
                        meshVerts[index] = (1.0f - row) * (1.0f - column) * verts[0]       // top-left corner
                                        + (1.0f - row) * column * verts[1]                 // top-right corner
                                        + row * column * verts[2]                          // bottom-right corner
                                        + row * (1.0f - column) * verts[3];                // bottom-left corner
                        if (colors != nullptr) {
                            meshColors[index] = (1.0f - row) * (1.0f - column) * colors[0]     // top-left corner
                                            + (1.0f - row) * column * colors[1]                // top-right corner
                                            + row * column * colors[2]                         // bottom-right corner
                                            + row * (1.0f - column) * colors[3];               // bottom-left corner
                    }
                        if (texs != nullptr) {
                            meshTexs[index] = (1.0f - row) * (1.0f - column) * texs[0]   // top-left corner
                                            + (1.0f - row) * column * texs[1]            // top-right corner
                                            + row * column * texs[2]                     // bottom-right corner
                                            + row * (1.0f - column) * texs[3];           // bottom-left corner
                        }
                    }
                }
                drawMesh(meshVerts, colors ? meshColors : nullptr, texs ? meshTexs : nullptr, 2, localIndices, paint);
            }
        }
    }

    void save() override {
       ctmStack.push(ctmStack.top());
    }

    void restore() override {
        if(!ctmStack.empty()){
            ctmStack.pop();
        }
    }

    void concat(const GMatrix& matrix) override {
        GMatrix currentMatrix = ctmStack.top();
        ctmStack.top() = GMatrix::Concat(currentMatrix, matrix);
    }

private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;
    std::stack<GMatrix> ctmStack;
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    canvas->clear({0.07f, 0.08f, 0.27f, 1.0f});
    GPaint starPaint;
    starPaint.setColor({0.99f, 0.99f, 0.4f, 1.0f});

    GPath star;
    GPoint starPoints[] = {
        {27.5, 3},
        {34, 19},
        {51, 19},
        {38, 30},
        {42, 48},
        {27.5, 38},
        {13, 48},
        {17, 30},
        {3, 19},
        {21, 19}
    };
    star.addPolygon(starPoints, 10);
    canvas->drawPath(star, starPaint);

    GPath star1;
    GPoint starPoints1[] = {
        {27.5 + 60, 3 + 30},
        {34 + 60, 19 + 30},
        {51 + 60, 19 + 30},
        {38 + 60, 30 + 30},
        {42 + 60, 48 + 30},
        {27.5 + 60, 38 + 30},
        {13 + 60, 48 + 30},
        {17 + 60, 30 + 30},
        {3 + 60, 19 + 30},
        {21 + 60, 19 + 30}
    };
    star1.addPolygon(starPoints1, 10);
    canvas->drawPath(star1, starPaint);

    GPath star2;
    GPoint starPoints2[] = {
        {27.5 + 25, 3 + 70},
        {34 + 25, 19 + 70},
        {51 + 25, 19 + 70},
        {38 + 25, 30 + 70},
        {42 + 25, 48 + 70},
        {27.5 + 25, 38 + 70},
        {13 + 25, 48 + 70},
        {17 + 25, 30 + 70},
        {3 + 25, 19 + 70},
        {21 + 25, 19 + 70}
    };
    star2.addPolygon(starPoints2, 10);
    canvas->drawPath(star2, starPaint); 

    GPath star3;
    GPoint starPoints3[] = {
        {27.5 + 115, 3 + 55},
        {34 + 115, 19 + 55},
        {51 + 115, 19 + 55},
        {38 + 115, 30 + 55},
        {42 + 115, 48 + 55},
        {27.5 + 115, 38 + 55},
        {13 + 115, 48 + 55},
        {17 + 115, 30 + 55},
        {3 + 115, 19 + 55},
        {21 + 115, 19 + 55}
    };
    star3.addPolygon(starPoints3, 10);
    canvas->drawPath(star3, starPaint);   

    GPath star4;
    GPoint starPoints4[] = {
        {27.5 + 188, 3 + 17},
        {34 + 188, 19 + 17},
        {51 + 188, 19 + 17},
        {38 + 188, 30 + 17},
        {42 + 188, 48 + 17},
        {27.5 + 188, 38 + 17},
        {13 + 188, 48 + 17},
        {17 + 188, 30 + 17},
        {3 + 188, 19 + 17},
        {21 + 188, 19 + 17}
    };
    star4.addPolygon(starPoints4, 10);
    canvas->drawPath(star4, starPaint);

    GPath star5;
    GPoint starPoints5[] = {
        {27.5 + 165, 3 + 107},
        {34 + 165, 19 + 107},
        {51 + 165, 19 + 107},
        {38 + 165, 30 + 107},
        {42 + 165, 48 + 107},
        {27.5 + 165, 38 + 107},
        {13 + 165, 48 + 107},
        {17 + 165, 30 + 107},
        {3 + 165, 19 + 107},
        {21 + 165, 19 + 107}
    };
    star5.addPolygon(starPoints5, 10);
    canvas->drawPath(star5, starPaint);

    GPath star6;
    GPoint starPoints6[] = {
        {27.5 + 73, 3 + 137},
        {34 + 73, 19 + 137},
        {51 + 73, 19 + 137},
        {38 + 73, 30 + 137},
        {42 + 73, 48 + 137},
        {27.5 + 73, 38 + 137},
        {13 + 73, 48 + 137},
        {17 + 73, 30 + 137},
        {3 + 73, 19 + 137},
        {21 + 73, 19 + 137}
    };
    star6.addPolygon(starPoints6, 10);
    canvas->drawPath(star6, starPaint);

    GPath star7;
    GPoint starPoints7[] = {
        {27.5 + 195, 3 + 157},
        {34 + 195, 19 + 157},
        {51 + 195, 19 + 157},
        {38 + 195, 30 + 157},
        {42 + 195, 48 + 157},
        {27.5 + 195, 38 + 157},
        {13 + 195, 48 + 157},
        {17 + 195, 30 + 157},
        {3 + 195, 19 + 157},
        {21 + 195, 19 + 157}
    };
    star7.addPolygon(starPoints7, 10);
    canvas->drawPath(star7, starPaint);

    GPath star8;
    GPoint starPoints8[] = {
        {27.5 + 120, 3 + 184},
        {34 + 120, 19 + 184},
        {51 + 120, 19 + 184},
        {38 + 120, 30 + 184},
        {42 + 120, 48 + 184},
        {27.5 + 120, 38 + 184},
        {13 + 120, 48 + 184},
        {17 + 120, 30 + 184},
        {3 + 120, 19 + 184},
        {21 + 120, 19 + 184}
    };
    star8.addPolygon(starPoints8, 10);
    canvas->drawPath(star8, starPaint);

    GPath star9;
    GPoint starPoints9[] = {
        {27.5 + 11, 3 + 186},
        {34 + 11, 19 + 186},
        {51 + 11, 19 + 186},
        {38 + 11, 30 + 186},
        {42 + 11, 48 + 186},
        {27.5 + 11, 38 + 186},
        {13 + 11, 48 + 186},
        {17 + 11, 30 + 186},
        {3 + 11, 19 + 186},
        {21 + 11, 19 + 186}
    };
    star9.addPolygon(starPoints9, 10);
    canvas->drawPath(star9, starPaint);

    return "A Sky Full of Stars";
}