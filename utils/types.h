#ifndef _TYPES_H
#define _TYPES_H

typedef char unsigned u8;
typedef short unsigned u16;
typedef int unsigned u32;
typedef long unsigned u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long s64;

typedef float f32;
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
    f32 data[3];
} f32x3;

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
    s32 data[3];
} s32x3;

#endif  // _TYPES_H
