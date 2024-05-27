#include "include/GMatrix.h"
#include "include/GShader.h"

class ComposeShader : public GShader {
public:
    ComposeShader(GShader* s1, GShader* s2) : shader1(s1), shader2(s2){}

    bool isOpaque() override {
        return shader1 -> isOpaque() && shader2 -> isOpaque();
    }

    bool setContext(const GMatrix& ctm) override {
        return shader1 -> setContext(ctm) && shader2 -> setContext(ctm);
    }

    inline unsigned division(unsigned num){
        return (num + 128) * 257 >> 16;
    }

    GPixel compose(GPixel p1, GPixel p2){
        unsigned a = division(GPixel_GetA(p1) * GPixel_GetA(p2));
        unsigned r = division(GPixel_GetR(p1) * GPixel_GetR(p2));
        unsigned g = division(GPixel_GetG(p1) * GPixel_GetG(p2));
        unsigned b = division(GPixel_GetB(p1) * GPixel_GetB(p2));
        return GPixel_PackARGB(a, r, g, b);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override{
        GPixel row1[count];
        GPixel row2[count];
        shader1 -> shadeRow(x, y, count, row1);
        shader2 -> shadeRow(x, y, count, row2);
        for(int i = 0; i < count; ++i){
            row[i] = compose(row1[i], row2[i]);
        }
    }

private:
    GShader* shader1;
    GShader* shader2;
};

std::unique_ptr<GShader> GCreateComposeShader(GShader* s1, GShader* s2){
    return std::unique_ptr<GShader>(new ComposeShader(s1, s2));
}