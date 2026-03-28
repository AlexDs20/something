#ifndef ADS_TYPES_H
#define ADS_TYPES_H
#include <stdint.h>
#include <stdbool.h>

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

typedef struct {
    f32 x, y;
} f32x2;

typedef struct {
    f32 x, y, z;
} f32x3;

typedef struct ALIGNAS(16) {
    f32 x, y, z, w;
} f32x4;

typedef union ALIGNAS(16) {
    float data[16];
    float m[4][4];
    struct {
        f32 m00, m01, m02, m03;
        f32 m10, m11, m12, m13;
        f32 m20, m21, m22, m23;
        f32 m30, m31, m32, m33;
    } elem;
    struct {
        f32x4 row0;
        f32x4 row1;
        f32x4 row2;
        f32x4 row3;
    } rows;
} f32x4x4;

typedef struct ALIGNAS(16) {
    f32x4 d;
} Quaternion;

#endif  // ADS_TYPES_H
