#ifndef ADS_TYPES_H
#define ADS_TYPES_H
#include <stdint.h>

#ifdef ADS_SSE
#include <xmmintrin.h>
#endif

#include "base/defines.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

typedef union {
    f32 f;
    u32 u;
} f32Bits;

typedef union {
    struct {
        f32 x, y;
    };
    struct {
        f32 u, v;
    };
    f32 data[2];
} f32x2;

typedef union {
    struct {
        f32 x, y, z;
    };
    struct {
        f32 u, v, w;
    };
    struct {
        f32 r, g, b;
    };
    f32 data[3];
} f32x3;

typedef union ALIGN(16) {
    struct { f32 x, y, z, w; };
    struct { f32 r, g, b, a; };
    f32 data[4];
#ifdef ADS_SSE
    __m128 v;
#endif
} f32x4;

typedef union ALIGN(16) {
    float data[16];
    float m4[4][4];
    struct {
        f32 m00, m01, m02, m03;
        f32 m10, m11, m12, m13;
        f32 m20, m21, m22, m23;
        f32 m30, m31, m32, m33;
    };
    struct {
        f32x4 row0;
        f32x4 row1;
        f32x4 row2;
        f32x4 row3;
    };
} f32x4x4;

typedef union {
    struct {
        f32 x, y, z, w;
    };
    f32 data[4];
} Quaternion;

#endif  // ADS_TYPES_H
