#include "include/GColor.h"
#include "include/GPixel.h"
#include "include/GMath.h"

struct Edge{
    float m, b, topx, ex;
    int ymin, ymax, wind;
    Edge(GPoint top, GPoint bottom, int w){
        if(top.y > bottom.y){
            std::swap(top, bottom);
        }
        ymin = GRoundToInt(top.y);
        ymax = GRoundToInt(bottom.y);
        if(ymin == ymax){
            return;
        }
        m = (top.x - bottom.x) / (top.y - bottom.y);
        topx = m * (ymin - top.y + 0.5f) + top.x;
        b = top.x - m * top.y;
        wind = w;
        ex = topx;
    }
    int computeX(int y){
        return GRoundToInt(m * (y + 0.5f) + b);
    }
    bool isValid(int y) {
        if(y >= ymin && y < ymax){
            return true;
        }
        return false;
    }
};

inline bool compareEdges(const Edge& e1, const Edge& e2) {
    if (e1.ymin != e2.ymin) {
        return e1.ymin > e2.ymin;
    }
    if (e1.topx != e2.topx) {
        return e1.topx > e2.topx;
    }
    return e1.m > e2.m;
}

inline bool comparePath(const Edge& e1, const Edge& e2){
    if (e1.ymin != e2.ymin) {
        return e1.ymin < e2.ymin;
    }
    if (e1.topx != e2.topx) {
        return e1.topx < e2.topx;
    }
    return e1.m < e2.m;
}

inline bool compareX(const Edge& e1, const Edge& e2){
    return e1.ex < e2.ex;
}

inline void clip(GRect rect, GPoint top, GPoint bottom, std::vector<Edge>& edges){
    int w = 1;
    if(top.y > bottom.y){
        std::swap(top, bottom);
        w = -1;
    }
    if(bottom.y <= rect.top || top.y >= rect.bottom || GRoundToInt(top.y) == GRoundToInt(bottom.y)){
        return;
    }
    if(top.y < rect.top){
        top.x += (bottom.x - top.x) * (rect.top - top.y) / (bottom.y - top.y);
        top.y = rect.top;
    }
    if(bottom.y > rect.bottom){
        bottom.x -= (bottom.x - top.x) * (bottom.y - rect.bottom) / (bottom.y - top.y);
        bottom.y = rect.bottom;
    }
    
    if(top.x > bottom.x){
        std::swap(top, bottom);
    }

    if(top.x < rect.left){
        if(bottom.x < rect.left){
            top.x = rect.left;
            bottom.x = rect.left;
        } else{
            GPoint ver;
            ver.x = rect.left;
            ver.y = top.y;
            top.y += (bottom.y - top.y) * (rect.left - top.x) / (bottom.x - top.x);
            top.x = rect.left;
            if (GRoundToInt(top.y) != GRoundToInt(ver.y)){
                Edge edge1(top, ver, w);
                edges.push_back(edge1);
            }
        }
    }
    if(top.x > rect.right){
        if(bottom.x > rect.right){
            top.x = rect.right;
            bottom.x = rect.right;
        } else{
            GPoint ver;
            ver.x = rect.right;
            ver.y = bottom.y;
            bottom.y -= (bottom.y - top.y) * (bottom.x - rect.right) / (bottom.x - top.x);
            bottom.x = rect.right;
            if (GRoundToInt(bottom.y) != GRoundToInt(ver.y)){
                Edge edge2(bottom, ver, w);
                edges.push_back(edge2);
            }
        }
    }
    if (GRoundToInt(top.y) != GRoundToInt(bottom.y)){
        Edge edge3(top, bottom, w);
        edges.push_back(edge3);
    }

}

inline std::vector<Edge> createPloygon(const GPoint points[], int count, GRect canvas){
    std::vector<Edge> edges;
    edges.reserve(count * 3);
    for(int i = 0; i < count; ++i){
        clip(canvas, points[i], points[(i + 1) % count], edges);
    }
    std::sort(edges.begin(), edges.end(), compareEdges);
    return edges;
}
