#include "include/GMatrix.h"
#include "include/GShader.h"

class TriGradient : public GShader {
public:
    TriGradient(const GColor colors[3], const GPoint points[3]){
        c0 = colors[0];
        c1 = colors[1];
        c2 = colors[2];
        GPoint u = points[1] - points[0];
        GPoint v = points[2] - points[0];
        dc1 = colors[1] - colors[0];
        dc2 = colors[2] - colors[0];
        //  M = | Ux   Vx  P0x |
        //      | Uy   Vy  P0y |
        //      |  0    0   1 |
        fLocalMatrix = GMatrix(u.x, v.x, points[0].x, u.y, v.y, points[0].y);
        fCtmInverse = GMatrix();
    }

    bool isOpaque() override {
        return (c0.a == 1.0f && c1.a == 1.0f && c2.a == 1.0f);
   }

    bool setContext(const GMatrix& ctm) override {
        return (ctm * fLocalMatrix).invert(&fCtmInverse);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override{
        GPoint deviceP = {x + 0.5f, y + 0.5f};
        GPoint gradientP = fCtmInverse * deviceP;
        // C = P’x * DC1 + P’y * DC2 + C0
        GColor color = gradientP.x * dc1 + gradientP.y * dc2 + c0;
        // DC = a * DC1 + d * DC2
        GColor dc = fCtmInverse[0] * dc1 + fCtmInverse[3] * dc2;
        for(int i = 0; i < count; ++i){
            row[i] = colorToPixel(color);
            color += dc;
        }
    }

private:
    GColor color[3];
    GColor dc1;
    GColor dc2;
    GColor c0;
    GColor c1;
    GColor c2;
    GMatrix fLocalMatrix;
    GMatrix fCtmInverse;
};

std::unique_ptr<GShader> GCreateTriangleGradient(GColor colors[3], GPoint points[3]){
    return std::unique_ptr<GShader>(new TriGradient(colors, points));
}