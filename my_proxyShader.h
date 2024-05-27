#include "include/GMatrix.h"
#include "include/GShader.h"

class ProxyShader : public GShader {
public:
    ProxyShader(const GPoint points[], const GPoint texs[], GShader* shader){
        // P = | Ux Vx P0x | = | P1x-P0x  P2x-P0x  P0x |
        //     | Uy Vy P0y |   | P1y-P0y  P2y-P0y  P0y |
        //     |  0  0  1  |   |    0        0      1  |

        GMatrix P = GMatrix(points[1].x - points[0].x, points[2].x - points[0].x, points[0].x,
                            points[1].y - points[0].y, points[2].y - points[0].y, points[0].y);
        // T = | Ux Vx T0x | = | T1x-T0x  T2x-T0x  T0x |
        //     | Uy Vy T0y |   | T1y-T0y  T2y-T0y  T0y |
        //     |  0  0  1  |   |    0        0      1  |
        GMatrix T = GMatrix(texs[1].x - texs[0].x, texs[2].x - texs[0].x, texs[0].x,
                            texs[1].y - texs[0].y, texs[2].y - texs[0].y, texs[0].y);
        GMatrix inverseT;
        T.invert(&inverseT);
        localTransform = P * inverseT;
        origShader = shader;
    }

    bool isOpaque() override {
        return origShader -> isOpaque();
   }

    bool setContext(const GMatrix& ctm) override {
        return origShader -> setContext(ctm * localTransform);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override{
        origShader -> shadeRow(x, y, count, row);
    }

private:
    GShader* origShader;
    GMatrix localTransform;
};

std::unique_ptr<GShader> GCreateProxyShader(const GPoint points[], const GPoint texs[], GShader* shader){
    return std::unique_ptr<GShader>(new ProxyShader(points, texs, shader));
}