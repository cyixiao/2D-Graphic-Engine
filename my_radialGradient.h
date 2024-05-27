#include "include/GMatrix.h"
#include "include/GShader.h"
#include "include/GBitmap.h"
#include <vector>
#include <cmath>

class RadialGradientShader : public GShader {
public:
    RadialGradientShader(GPoint center, float radius, const GColor colors[], int count, GShader::TileMode tileMode) 
        : colorNum(count), mode(tileMode), center(center), radius(radius) {
        for (int i = 0; i < count; ++i) {
            this->colors.push_back(colors[i]);
        }
        for (int i = 0; i < count - 1; ++i) {
            this->colorsDiff.push_back(colors[i + 1] - colors[i]);
        }
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
        return ctm.invert(&fCtmInverse);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint deviceP = {x + 0.5f, y + 0.5f};
        GPoint gradientP = fCtmInverse * deviceP;
        for (int i = 0; i < count; ++i) {
            float dx = gradientP.x - center.x;
            float dy = gradientP.y - center.y;
            float distance = std::sqrt(dx * dx + dy * dy) / radius;
            float temp = makeTile(distance);
            int index = GFloorToInt(temp * (colorNum - 1));
            float t = temp * (colorNum - 1) - index;
            if (t == 0) {
                row[i] = color2Pixel(colors[index]);
            } else {
                row[i] = color2Pixel(colors[index] + colorsDiff[index] * t);
            }
            gradientP.x += fCtmInverse[0];
            gradientP.y += fCtmInverse[3];
        }
    }

    GPixel color2Pixel(const GColor& color){
        GColor colort = color.pinToUnit();
        unsigned r = GRoundToInt(colort.r * colort.a * 255);
        unsigned g = GRoundToInt(colort.g * colort.a * 255);
        unsigned b = GRoundToInt(colort.b * colort.a * 255);
        unsigned a = GRoundToInt(colort.a * 255);
        return GPixel_PackARGB(a, r, g, b);
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
    GMatrix fCtmInverse;
    GShader::TileMode mode;
    GPoint center;
    float radius;
};
