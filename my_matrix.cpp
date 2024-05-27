#include "include/GMath.h"
#include "include/GPoint.h"
#include "include/GMatrix.h"

GMatrix::GMatrix() : GMatrix(1, 0, 0, 0, 1, 0){}

GMatrix GMatrix::Translate(float tx, float ty){
    return GMatrix(1, 0, tx, 0, 1, ty);
}

GMatrix GMatrix::Scale(float sx, float sy){
    return GMatrix(sx, 0, 0, 0, sy, 0);
}

GMatrix GMatrix::Rotate(float radians){
    return GMatrix(
        cos(radians), 
        -sin(radians), 
        0, 
        sin(radians),
        cos(radians), 
        0);
}

GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b){
    return GMatrix(
        a[0] * b[0] + a[1] * b[3], 
        a[0] * b[1] + a[1] * b[4], 
        a[0] * b[2] + a[1] * b[5] + a[2],
        a[3] * b[0] + a[4] * b[3], 
        a[3] * b[1] + a[4] * b[4], 
        a[3] * b[2] + a[4] * b[5] + a[5]
    );
}

bool GMatrix::invert(GMatrix* inverse) const {
    float det = fMat[0] * fMat[4] - fMat[1] * fMat[3];
    if (det == 0) {
        return false;
    }
    float invDet = 1 / det;
    *inverse = GMatrix(
        fMat[4] * invDet, 
        -fMat[1] * invDet, 
        (fMat[1] * fMat[5] - fMat[2] * fMat[4]) * invDet,
        -fMat[3] * invDet, 
        fMat[0] * invDet, 
        (fMat[2] * fMat[3] - fMat[0] * fMat[5]) * invDet
    );
    return true;
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; ++i) {
        float x = src[i].x;
        float y = src[i].y;
        dst[i].x = fMat[0] * x + fMat[1] * y + fMat[2];
        dst[i].y = fMat[3] * x + fMat[4] * y + fMat[5];
    }
}