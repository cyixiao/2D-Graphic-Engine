#include "include/GColor.h"
#include "include/GPixel.h"
#include "include/GBitmap.h"
#include "include/GMath.h"
#include "include/GBlendMode.h"

GPixel colorToPixel(const GColor& color){
    GColor colort = color.pinToUnit();
    unsigned r = GRoundToInt(colort.r * colort.a * 255);
    unsigned g = GRoundToInt(colort.g * colort.a * 255);
    unsigned b = GRoundToInt(colort.b * colort.a * 255);
    unsigned a = GRoundToInt(colort.a * 255);
    return GPixel_PackARGB(a, r, g, b);
}

inline unsigned blendDivision(unsigned num){
    return (num + 128) * 257 >> 16;
}

inline GPixel kClear(GPixel src, GPixel dst){
    return GPixel_PackARGB(0, 0, 0, 0);
}

inline GPixel kSrc(GPixel src, GPixel dst){
    return src;
}

inline GPixel kDst(GPixel src, GPixel dst){
    return dst;
}

inline GPixel kSrcOver(GPixel src, GPixel dst){
    unsigned a, r, g, b;
    r = GPixel_GetR(src) + blendDivision((255 - GPixel_GetA(src)) * GPixel_GetR(dst));
    g = GPixel_GetG(src) + blendDivision((255 - GPixel_GetA(src)) * GPixel_GetG(dst));
    b = GPixel_GetB(src) + blendDivision((255 - GPixel_GetA(src)) * GPixel_GetB(dst));
    a = GPixel_GetA(src) + blendDivision((255 - GPixel_GetA(src)) * GPixel_GetA(dst));
    return GPixel_PackARGB(a, r, g, b);
    
}

inline GPixel kDstOver(GPixel src, GPixel dst){
    unsigned a, r, g, b;
    r = GPixel_GetR(dst) + blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetR(src));
    g = GPixel_GetG(dst) + blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetG(src));
    b = GPixel_GetB(dst) + blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetB(src));
    a = GPixel_GetA(dst) + blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetA(src));
    return GPixel_PackARGB(a, r, g, b);

}

inline GPixel kSrcIn(GPixel src, GPixel dst){
    unsigned a, r, g, b;

    r = blendDivision(GPixel_GetA(dst) * GPixel_GetR(src));
    g = blendDivision(GPixel_GetA(dst) * GPixel_GetG(src));
    b = blendDivision(GPixel_GetA(dst) * GPixel_GetB(src));
    a = blendDivision(GPixel_GetA(dst) * GPixel_GetA(src));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel kDstIn(GPixel src, GPixel dst){
    unsigned a, r, g, b;
    r = blendDivision(GPixel_GetA(src) * GPixel_GetR(dst));
    g = blendDivision(GPixel_GetA(src) * GPixel_GetG(dst));
    b = blendDivision(GPixel_GetA(src) * GPixel_GetB(dst));
    a = blendDivision(GPixel_GetA(src) * GPixel_GetA(dst));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel kSrcOut(GPixel src, GPixel dst){
    unsigned a, r, g, b;

    r = blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetR(src));
    g = blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetG(src));
    b = blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetB(src));
    a = blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetA(src));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel kDstOut(GPixel src, GPixel dst){
    unsigned a, r, g, b;

    r = blendDivision((255 - GPixel_GetA(src)) * GPixel_GetR(dst));
    g = blendDivision((255 - GPixel_GetA(src)) * GPixel_GetG(dst));
    b = blendDivision((255 - GPixel_GetA(src)) * GPixel_GetB(dst));
    a = blendDivision((255 - GPixel_GetA(src)) * GPixel_GetA(dst));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel kSrcATop(GPixel src, GPixel dst){
    unsigned a, r, g, b;
    a = blendDivision(GPixel_GetA(dst) * GPixel_GetA(src)) + blendDivision((255 - GPixel_GetA(src)) * GPixel_GetA(dst));
    r = blendDivision(GPixel_GetA(dst) * GPixel_GetR(src)) + blendDivision((255 - GPixel_GetA(src)) * GPixel_GetR(dst));
    g = blendDivision(GPixel_GetA(dst) * GPixel_GetG(src)) + blendDivision((255 - GPixel_GetA(src)) * GPixel_GetG(dst));
    b = blendDivision(GPixel_GetA(dst) * GPixel_GetB(src)) + blendDivision((255 - GPixel_GetA(src)) * GPixel_GetB(dst));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel kDstATop(GPixel src, GPixel dst){
    unsigned a, r, g, b;
    a = blendDivision(GPixel_GetA(dst) * GPixel_GetA(src)) + blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetA(src));
    r = blendDivision(GPixel_GetR(dst) * GPixel_GetA(src)) + blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetR(src));
    g = blendDivision(GPixel_GetG(dst) * GPixel_GetA(src)) + blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetG(src));
    b = blendDivision(GPixel_GetB(dst) * GPixel_GetA(src)) + blendDivision((255 - GPixel_GetA(dst)) * GPixel_GetB(src));
    return GPixel_PackARGB(a, r, g, b);
}

inline GPixel kXor(GPixel src, GPixel dst){
    unsigned a, r, g, b;
    a = blendDivision((255 - GPixel_GetA(src)) * GPixel_GetA(dst) + (255 - GPixel_GetA(dst)) * GPixel_GetA(src));
    r = blendDivision((255 - GPixel_GetA(src)) * GPixel_GetR(dst) + (255 - GPixel_GetA(dst)) * GPixel_GetR(src));
    g = blendDivision((255 - GPixel_GetA(src)) * GPixel_GetG(dst) + (255 - GPixel_GetA(dst)) * GPixel_GetG(src));
    b = blendDivision((255 - GPixel_GetA(src)) * GPixel_GetB(dst) + (255 - GPixel_GetA(dst)) * GPixel_GetB(src));
    return GPixel_PackARGB(a, r, g, b);
}

typedef GPixel (*Basic)(GPixel, GPixel);

// blend with shader

inline void sClear(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = GPixel_PackARGB(0, 0, 0, 0);
    }
}

inline void sSrc(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = source[i - tempLeft];
    }
}

inline void sDst(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    return;
}

inline void sSrcOver(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kSrcOver(source[i - tempLeft], *addr);
    }
}

inline void sDstOver(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kDstOver(source[i - tempLeft], *addr);
    }
}

inline void sSrcIn(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kSrcIn(source[i - tempLeft], *addr);
    }
}

inline void sDstIn(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kDstIn(source[i - tempLeft], *addr);
    }
}

inline void sSrcOut(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kSrcOut(source[i - tempLeft], *addr);
    }
}

inline void sDstOut(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kDstOut(source[i - tempLeft], *addr);
    }
}

inline void sSrcATop(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kSrcATop(source[i - tempLeft], *addr);
    }
}

inline void sDstATop(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kDstATop(source[i - tempLeft], *addr);
    }
}

inline void sXor(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source[]){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kXor(source[i - tempLeft], *addr);
    }
}

typedef void (*BlendingSha)(const GBitmap&, int, int, int, GPixel[]);

BlendingSha optimizingSha(GBlendMode mode){
    switch (mode){
        case GBlendMode::kClear:
        return sClear;
        case GBlendMode::kSrc:
        return sSrc;
        case GBlendMode::kDst:
        return sDst;
        case GBlendMode::kSrcOver:
        return sSrcOver;
        case GBlendMode::kDstOver:
        return sDstOver;
        case GBlendMode::kSrcIn:
        return sSrcIn;
        case GBlendMode::kDstIn:
        return sDstIn;
        case GBlendMode::kSrcOut:
        return sSrcOut;
        case GBlendMode::kDstOut:
        return sDstOut;
        case GBlendMode::kSrcATop:
        return sSrcATop;
        case GBlendMode::kDstATop:
        return sDstATop;
        case GBlendMode::kXor:
        return sXor;
        default:
        return sClear;
    }
}

// blend without shader
inline void nClear(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = GPixel_PackARGB(0, 0, 0, 0);
    }
}

inline void nSrc(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = source;
    }
}

inline void nDst(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    return;
}

inline void nSrcOver(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kSrcOver(source, *addr);
    }
}

inline void nDstOver(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kDstOver(source, *addr);
    }
}

inline void nSrcIn(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kSrcIn(source, *addr);
    }
}

inline void nDstIn(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kDstIn(source, *addr);
    }
}

inline void nSrcOut(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kSrcOut(source, *addr);
    }
}

inline void nDstOut(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kDstOut(source, *addr);
    }
}

inline void nSrcATop(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kSrcATop(source, *addr);
    }
}

inline void nDstATop(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kDstATop(source, *addr);
    }
}

inline void nXor(const GBitmap& device, int tempLeft, int tempRight, int y, GPixel source){
    for(int i = tempLeft; i < tempRight; ++i){
        GPixel* addr = device.getAddr(i, y);
        *addr = kXor(source, *addr);
    }
}

typedef void (*BlendingNor)(const GBitmap&, int, int, int, GPixel);

BlendingNor optimizingNor(GBlendMode mode, unsigned sa){
    switch (mode){
        case GBlendMode::kClear:
        return nClear;
        case GBlendMode::kSrc:
        return nSrc;
        case GBlendMode::kDst:
        return nDst;
        case GBlendMode::kSrcOver:
        switch (sa){
            case 0:
            return nDst;
            case 255:
            return nSrc;
            default:
            return nSrcOver;
        }
        case GBlendMode::kDstOver:
        switch (sa){
            case 0:
            return nDst;
            default:
            return nDstOver;
        }
        case GBlendMode::kSrcIn:
        switch (sa){
            case 0:
            return nClear;
            default:
            return nSrcIn;
        }
        case GBlendMode::kDstIn:
        switch (sa){
            case 0:
            return nClear;
            case 255:
            return nDst;
            default:
            return nDstIn;
        }
        case GBlendMode::kSrcOut:
        switch (sa){
            case 0:
            return nClear;
            default:
            return nSrcOut;
        }
        case GBlendMode::kDstOut:
        switch (sa){
            case 0:
            return nDst;
            case 255:
            return nClear;
            default:
            return nDstOut;
        }
        case GBlendMode::kSrcATop:
        switch (sa){
            case 0:
            return nDst;
            case 255:
            return nSrcIn;
            default:
            return nSrcATop;
        }
        case GBlendMode::kDstATop:
        switch (sa){
            case 0:
            return nClear;
            case 255:
            return nDstOver;
            default:
            return nDstATop;
        }
        case GBlendMode::kXor:
        switch (sa){
            case 0:
            return nDst;
            case 255:
            return nSrcOut;
            default:
            return nXor;
        }
        default:
            return nClear;
    }
}

// blend for rectangle

inline void Clear(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            *device.getAddr(j, i) = GPixel_PackARGB(0, 0, 0, 0);
        }
    }
}

inline void Src(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            *device.getAddr(j, i) = source;
        }
    }
}

inline void Dst(const GBitmap& device, GIRect rectp, GPixel source){
    return;
}

inline void SrcOver(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            GPixel* p = device.getAddr(j, i);
            *p = kSrcOver(source, *p);
        }
    }
}

inline void DstOver(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            GPixel* p = device.getAddr(j, i);
            *p = kDstOver(source, *p);
        }
    }
}

inline void SrcIn(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            GPixel* p = device.getAddr(j, i);
            *p = kSrcIn(source, *p);
        }
    }
}

inline void DstIn(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            GPixel* p = device.getAddr(j, i);
            *p = kDstIn(source, *p);
        }
    }
}

inline void SrcOut(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            GPixel* p = device.getAddr(j, i);
            *p = kSrcOut(source, *p);
        }
    }
}

inline void DstOut(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            GPixel* p = device.getAddr(j, i);
            *p = kDstOut(source, *p);
        }
    }
}

inline void SrcATop(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            GPixel* p = device.getAddr(j, i);
            *p = kSrcATop(source, *p);
        }
    }
}

inline void DstATop(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            GPixel* p = device.getAddr(j, i);
            *p = kDstATop(source, *p);
        }
    }
}

inline void Xor(const GBitmap& device, GIRect rectp, GPixel source){
    for(int i = rectp.top; i < rectp.bottom; ++i){
        for(int j = rectp.left; j < rectp.right; ++j){
            GPixel* p = device.getAddr(j, i);
            *p = kXor(source, *p);
        }
    }
}

typedef void (*BlendingFor)(const GBitmap&, GIRect, GPixel);

BlendingFor optimizing(GBlendMode mode, unsigned sa){
    switch (mode){
        case GBlendMode::kClear:
        return Clear;
        case GBlendMode::kSrc:
        return Src;
        case GBlendMode::kDst:
        return Dst;
        case GBlendMode::kSrcOver:
        switch (sa){
            case 0:
            return Dst;
            case 255:
            return Src;
            default:
            return SrcOver;
        }
        case GBlendMode::kDstOver:
        switch (sa){
            case 0:
            return Dst;
            default:
            return DstOver;
        }
        case GBlendMode::kSrcIn:
        switch (sa){
            case 0:
            return Clear;
            default:
            return SrcIn;
        }
        case GBlendMode::kDstIn:
        switch (sa){
            case 0:
            return Clear;
            case 255:
            return Dst;
            default:
            return DstIn;
        }
        case GBlendMode::kSrcOut:
        switch (sa){
            case 0:
            return Clear;
            default:
            return SrcOut;
        }
        case GBlendMode::kDstOut:
        switch (sa){
            case 0:
            return Dst;
            case 255:
            return Clear;
            default:
            return DstOut;
        }
        case GBlendMode::kSrcATop:
        switch (sa){
            case 0:
            return Dst;
            case 255:
            return SrcIn;
            default:
            return SrcATop;
        }
        case GBlendMode::kDstATop:
        switch (sa){
            case 0:
            return Clear;
            case 255:
            return DstOver;
            default:
            return DstATop;
        }
        case GBlendMode::kXor:
        switch (sa){
            case 0:
            return Dst;
            case 255:
            return SrcOut;
            default:
            return Xor;
        }
        default:
            return Clear;
    }
}

Basic Blender(GBlendMode mode){
    switch (mode){
        case GBlendMode::kClear:
        return kClear;
        case GBlendMode::kSrc:
        return kSrc;
        case GBlendMode::kDst:
        return kDst;
        case GBlendMode::kSrcOver:
        return kSrcOver;
        case GBlendMode::kDstOver:
        return kDstOver;
        case GBlendMode::kSrcIn:
        return kSrcIn;
        case GBlendMode::kDstIn:
        return kDstIn;
        case GBlendMode::kSrcOut:
        return kSrcOut;
        case GBlendMode::kDstOut:
        return kDstOut;
        case GBlendMode::kSrcATop:
        return kSrcATop;
        case GBlendMode::kDstATop:
        return kDstATop;
        case GBlendMode::kXor:
        return kXor;
        default:
        return kClear;
    }
}
