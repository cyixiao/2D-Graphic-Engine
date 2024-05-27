#include "include/GMatrix.h"
#include "include/GShader.h"
#include <vector>

GPixel color2Pixel(const GColor& color){
    GColor colort = color.pinToUnit();
    unsigned r = GRoundToInt(colort.r * colort.a * 255);
    unsigned g = GRoundToInt(colort.g * colort.a * 255);
    unsigned b = GRoundToInt(colort.b * colort.a * 255);
    unsigned a = GRoundToInt(colort.a * 255);
    return GPixel_PackARGB(a, r, g, b);
}

class MyGradient : public GShader {
public:
    MyGradient(GPoint p0, GPoint p1, const GColor color[], int count, GShader::TileMode tileMode) : colorNum(count), mode(tileMode){
        for(int i = 0; i < count; ++i){
            colors.push_back(color[i]);
        }
        for(int i = 0; i < count - 1; ++i){
            colorsDiff.push_back(colors[i + 1] - colors[i]);
        }
        float dx = p1.x - p0.x;
        float dy = p1.y - p0.y;
        fLocalMatrix = GMatrix(dx, -dy, p0.x, dy, dx, p0.y);
        fCtmInverse = GMatrix();
    }

    bool isOpaque() override {
        for(int i = 0; i < colorNum; ++i){
            if(colors[i].a != 1.0){
                return false;
            }
        }
        return true;
   }

    bool setContext(const GMatrix& ctm) override {
        return (ctm * fLocalMatrix).invert(&fCtmInverse);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override{
        GPoint deviceP = {x + 0.5f, y + 0.5f};
        GPoint gradientP = fCtmInverse * deviceP;
        if (colorNum == 1){
            std::fill(row, row + count, color2Pixel(colors[0]));
        } else{
            if (fCtmInverse[0] == 0 && fCtmInverse[3] == 0) {
                float tempX = makeTile(gradientP.x);
                int index = GFloorToInt(tempX * (colorNum - 1));
                float t = tempX * (colorNum - 1) - index;
                if(t == 0){
                    std::fill(row, row + count, color2Pixel(colors[index]));
                } else{
                    std::fill(row, row + count, color2Pixel(colors[index] + colorsDiff[index] * t));
                }
            } else{
                for (int i = 0; i < count; ++i){
                    float tempX = makeTile(gradientP.x);
                    int index = GFloorToInt(tempX * (colorNum - 1));
                    float t = tempX * (colorNum - 1) - index;
                    if(t == 0){
                        row[i] = color2Pixel(colors[index]);
                    } else{
                        row[i] = color2Pixel(colors[index] + colorsDiff[index] * t);
                    }
                    gradientP.x += fCtmInverse[0];
                    gradientP.y += fCtmInverse[3];
                }
            }
        }
    }

    inline float makeTile(float x){
        if(mode == GShader::TileMode::kClamp){
            return std::min(1.0f, std::max(0.0f, x));
        } else if(mode == GShader::TileMode::kRepeat){
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
    int colorNum;
    std::vector<GColor> colors;
    std::vector<GColor> colorsDiff;
    GMatrix fLocalMatrix;
    GMatrix fCtmInverse;
    GShader::TileMode mode;
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor color[], int count, GShader::TileMode tileMode){
    return std::unique_ptr<GShader>(new MyGradient(p0, p1, color, count, tileMode));
}