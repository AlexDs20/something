#ifndef _TYPES_H
#define _TYPES_H
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef union {
    u32 data;
    struct {
        u8 a, r, g, b;
    };
    struct {
        u8 _, Y, Cb, Cr;
    };
} u32Bytes;

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

typedef union {
    struct {
        u32 x, y;
    };
    struct {
        u32 u, v;
    };
    u32 data[2];
} u32x2;

typedef union {
    struct {
        u32 x, y, z;
    };
    struct {
        u32 u, v, w;
    };
    struct {
        u32 r, g, b;
    };
    struct {
        u32 Y, Cb, Cr;
    };
    u32 data[3];
} u32x3;

typedef union {
    struct {
        u32 a, r, g, b;
    };
    struct {
        u32 _, Y, Cb, Cr;
    };
    u32 data[4];
} u32x4;

typedef union {
    struct {
        s32 x, y;
    };
    struct {
        s32 u, v;
    };
    s32 data[2];
} s32x2;

typedef union {
    struct {
        s32 x, y, z;
    };
    struct {
        s32 u, v, w;
    };
    struct {
        s32 Y, Cb, Cr;
    };
    s32 data[3];
} s32x3;

struct string8 {
    u8* buffer;
    u64 size;
};

struct Image {
    u32 width;
    u32 height;
    u32* data;
};

#endif  // _TYPES_H
