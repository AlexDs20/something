#ifndef _DEFINES_H
#define _DEFINES_H

#if defined(__clang__)
#elif defined(__GNUC__)
#elif defined(_MSC_VER)
#elif defined(__INTEL_COMPILER)
#else
#endif

#define global_variable static
#define local_variable static
#define internal_function static
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define KiB (1024)               // KiB
#define MiB (1024*1024)          // MiB
#define GiB (1024*1024*1024)     // GiB

#define EPS (1e-6)

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

#endif // _DEFINES_H
