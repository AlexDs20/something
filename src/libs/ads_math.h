#ifndef _ADS_MATH_H_
#define _ADS_MATH_H_

#include "base/base.h"

// f32x2
f32x2 operator+(f32x2 a, f32x2 b);
f32x2 operator+(f32   a, f32x2 b);
f32x2 operator+(f32x2 a, f32   b);
f32x2 operator-(f32x2 a, f32x2 b);
f32x2 operator-(f32   a, f32x2 b);
f32x2 operator-(f32x2 a, f32   b);
f32x2 operator*(f32x2 a, f32x2 b);
f32x2 operator*(f32   a, f32x2 b);
f32x2 operator*(f32x2 a, f32   b);
f32x2 operator/(f32x2 a, f32x2 b);
f32x2 operator/(f32   a, f32x2 b);
f32x2 operator/(f32x2 a, f32   b);
bool  operator==(f32x2 a, f32x2 b);
bool  operator!=(f32x2 a, f32x2 b);
f32   length2_f32x2(f32x2 a);
f32   length_f32x2 (f32x2 a);
f32x2 normalize_f32x2(f32x2 a);
f32   dot_f32x2(f32x2 a, f32x2 b);
f32   cross_f32x2(f32x2 a, f32x2 b);
f32x2 perpendicular_f32x2(f32x2 a);

// f32x3
f32x3 operator+(f32x3 a, f32x3 b);
f32x3 operator+(f32   a, f32x3 b);
f32x3 operator+(f32x3 a, f32   b);
f32x3 operator-(f32x3 a, f32x3 b);
f32x3 operator-(f32   a, f32x3 b);
f32x3 operator-(f32x3 a, f32   b);
f32x3 operator*(f32x3 a, f32x3 b);
f32x3 operator*(f32   a, f32x3 b);
f32x3 operator*(f32x3 a, f32   b);
f32x3 operator/(f32x3 a, f32x3 b);
f32x3 operator/(f32   a, f32x3 b);
f32x3 operator/(f32x3 a, f32   b);
bool  operator==(f32x3 a, f32x3 b);
bool  operator!=(f32x3 a, f32x3 b);
f32   length2_f32x3(f32x3 a);
f32   length_f32x3 (f32x3 a);
f32x3 normalize_f32x3(f32x3 a);
f32   dot_f32x3(f32x3 a, f32x3 b);
f32   cross_f32x3(f32x3 a, f32x3 b);

// f32x4
typedef struct f32x4 f32x4;
struct {
    f32 data[4];
    struct {
        f32 x, y, z, w;
    }
};
f32x4 operator+(f32x4 a, f32x4 b);
f32x4 operator+(f32   a, f32x4 b);
f32x4 operator+(f32x4 a, f32   b);
f32x4 operator-(f32x4 a, f32x4 b);
f32x4 operator-(f32   a, f32x4 b);
f32x4 operator-(f32x4 a, f32   b);
f32x4 operator*(f32x4 a, f32x4 b);
f32x4 operator*(f32   a, f32x4 b);
f32x4 operator*(f32x4 a, f32   b);
f32x4 operator/(f32x4 a, f32x4 b);
f32x4 operator/(f32   a, f32x4 b);
f32x4 operator/(f32x4 a, f32   b);
bool  operator==(f32x4 a, f32x4 b);
bool  operator!=(f32x4 a, f32x4 b);
f32   length2_f32x4(f32x4 a);
f32   length_f32x4 (f32x4 a);
f32x4 normalize_f32x4(f32x4 a);
f32   dot_f32x4(f32x4 a, f32x4 b);
f32   cross_f32x4(f32x4 a, f32x4 b);

// Matrix 4x4
typedef union {
    float m[16];
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

// Quaternion
typedef union {
} Quaternion;

#endif // _ADS_MATH_H_
