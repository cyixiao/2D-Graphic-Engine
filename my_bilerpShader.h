#include "include/GMatrix.h"
#include "include/GShader.h"
#include "include/GBitmap.h"

class BilerpShader : public GShader {
public:
    BilerpShader(const GBitmap& bitmap, const GMatrix& localMatrix) : fBitmap(bitmap), fLocalMatrix(localMatrix) {
        fCtmInverse = GMatrix();
    }

    bool isOpaque() override {
        return fBitmap.isOpaque();
    }

    bool setContext(const GMatrix& ctm) override {
        GMatrix tempCtm;
        if(!ctm.invert(&tempCtm)){
            return false;
        }
        fCtmInverse = GMatrix::Concat(fLocalMatrix, tempCtm);
        return true;
    }

    GPixel color2Pixel(const GColor& color){
        GColor colort = color.pinToUnit();
        unsigned r = GRoundToInt(colort.r * colort.a * 255.f);
        unsigned g = GRoundToInt(colort.g * colort.a * 255.f);
        unsigned b = GRoundToInt(colort.b * colort.a * 255.f);
        unsigned a = GRoundToInt(colort.a * 255.f);
        return GPixel_PackARGB(a, r, g, b);
    }

    GColor pixel2Color(GPixel pixel){
        return GColor::RGBA(GPixel_GetR(pixel) / 255.f,
                            GPixel_GetG(pixel) / 255.f,
                            GPixel_GetB(pixel) / 255.f,
                            GPixel_GetA(pixel) / 255.f);
    }

    int clamp(int device, int point){
        return std::max(0, std::min(device - 1, point));
    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) override{
        GPoint deviceP = {x + 0.5f, y + 0.5f};
        GPoint bitMapP = fCtmInverse * deviceP;
        for(int i = 0; i < count; ++i){
            int sx = GRoundToInt(bitMapP.x);
            int sy = GRoundToInt(bitMapP.y);
            float vx = bitMapP.x + 0.5 - GRoundToInt(bitMapP.x);
            float vy = bitMapP.y + 0.5 - GRoundToInt(bitMapP.y);
            GColor color1 = pixel2Color(
                *fBitmap.getAddr(clamp(fBitmap.width(), sx - 1), clamp(fBitmap.height(), sy - 1)));
            GColor color2 = pixel2Color(
                *fBitmap.getAddr(clamp(fBitmap.width(), sx), clamp(fBitmap.height(), sy - 1)));
            GColor color3 = pixel2Color(
                *fBitmap.getAddr(clamp(fBitmap.width(), sx - 1), clamp(fBitmap.height(), sy)));
            GColor color4 = pixel2Color(
                *fBitmap.getAddr(clamp(fBitmap.width(), sx), clamp(fBitmap.height(), sy)));
            row[i] = color2Pixel((color1 * (1 - vx) + color2 * vx) * (1 - vy)
                                + (color3 * (1 - vx) + color4 * vx) * vy);
            bitMapP.x += fCtmInverse[0];
            bitMapP.y += fCtmInverse[3];
        }
    }
    
private:
    GBitmap fBitmap;
    GMatrix fLocalMatrix;
    GMatrix fCtmInverse;
};