#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include "include/GPoint.h"
#include "include/GMath.h"

class MyShader : public GShader {
public:
    MyShader(const GBitmap& bitmap, const GMatrix& localMatrix, GShader::TileMode tileMode) : fBitmap(bitmap), fLocalMatrix(localMatrix), mode(tileMode) {
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

    void shadeRow(int x, int y, int count, GPixel row[]) override{
        GPoint deviceP = {x + 0.5f, y + 0.5f};
        GPoint bitMapP = fCtmInverse * deviceP;

        if (fCtmInverse[0] == 0 && fCtmInverse[3] == 0) {
            if(mode == GShader::TileMode::kClamp){
                int sx = GFloorToInt(bitMapP.x);
                int sy = GFloorToInt(bitMapP.y);
                sx = std::min(std::max(0, sx), fBitmap.width() - 1);    
                sy = std::min(std::max(0, sy), fBitmap.height() - 1);
                GPixel pixel = *fBitmap.getAddr(sx, sy);
                std::fill(row, row + count, pixel);
            } else{
                float sx = bitMapP.x;
                float sy = bitMapP.y;
                sx = makeTile(sx / fBitmap.width()) * fBitmap.width();
                sy = makeTile(sy / fBitmap.height()) * fBitmap.height();
                GPixel pixel = *fBitmap.getAddr(GFloorToInt(sx), GFloorToInt(sy));
                std::fill(row, row + count, pixel);
            }
        } else{
            for(int i = 0; i < count; ++i){
                if(mode == GShader::TileMode::kClamp){
                    int sx = GFloorToInt(bitMapP.x);
                    int sy = GFloorToInt(bitMapP.y);
                    sx = std::min(std::max(0, sx), fBitmap.width() - 1);
                    sy = std::min(std::max(0, sy), fBitmap.height() - 1);
                    row[i] = *fBitmap.getAddr(sx,  sy);
                } else{
                    float sx = bitMapP.x;
                    float sy = bitMapP.y;
                    sx = makeTile(sx / fBitmap.width()) * fBitmap.width();
                    sy = makeTile(sy / fBitmap.height()) * fBitmap.height();
                    row[i] = *fBitmap.getAddr(GFloorToInt(sx),  GFloorToInt(sy));
                }
                bitMapP.x += fCtmInverse[0];
                bitMapP.y += fCtmInverse[3];
            }
        }
    }

    inline float makeTile(float x){
        if(mode == GShader::TileMode::kRepeat){
            return x - floor(x);
        } else{
            x *= 0.5f;
            float result = x - floor(x);
            if (result > 0.5) {
                result = (1 - result);
            }
            return 2 * result;
        }
    }

private:
    GBitmap fBitmap;
    GMatrix fLocalMatrix;
    GMatrix fCtmInverse;
    GShader::TileMode mode;
};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localInverse, GShader::TileMode tileMode){
    return std::unique_ptr<GShader>(new MyShader(bitmap, localInverse, tileMode));
}