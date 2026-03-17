#ifndef ADS_MATH_H
#define ADS_MATH_H
#include "base/base.h"

//====================
//      u8
//====================
u8 maxu8(u8 a, u8 b) ;
u8 minu8(u8 a, u8 b) ;
u8 clampu8(u8 a, u8 low, u8 high) ;

//====================
//      s8
//====================
s8 maxs8(s8 a, s8 b) ;
s8 mins8(s8 a, s8 b) ;
s8 clamps8(s8 a, s8 low, s8 high) ;

//====================
//      u16
//====================
u16 maxu16(u16 a, u16 b) ;
u16 minu16(u16 a, u16 b) ;
u16 clampu16(u16 a, u16 low, u16 high) ;

//====================
//      s16
//====================
s16 maxs16(s16 a, s8 b) ;
s16 mins16(s16 a, s8 b) ;
s16 clamps16(s16 a, s16 low, s16 high) ;

//====================
//      u32
//====================
u32 maxu32(u32 a, u32 b) ;
u32 minu32(u32 a, u32 b) ;
u32 clampu32(u32 a, u32 low, u32 high) ;

//====================
//      s32
//====================
s32 maxs32(s32 a, s32 b) ;
s32 mins32(s32 a, s32 b) ;
s32 clamps32(s32 a, s32 low, s32 high) ;

//====================
//      f32
//====================
f32 maxf32(f32 a, f32 b) ;
f32 minf32(f32 a, f32 b) ;
f32 clampf32(f32 a, f32 low, f32 high) ;
//  s32 ceilf32(f32 a);

//====================
//      u32x2
//====================
// +
u32x2 operator+(u32x2 a, u32x2 b) ;
u32x2 operator+(u32 a, u32x2 b) ;
u32x2 operator+(u32x2 a, u32 b) ;

// -
u32x2 operator-(u32x2 a, u32x2 b) ;
u32x2 operator-(u32 a, u32x2 b) ;
u32x2 operator-(u32x2 a, u32 b) ;

// *
u32x2 operator*(u32x2 a, u32x2 b) ;
u32x2 operator*(u32 a, u32x2 b) ;
u32x2 operator*(u32x2 a, u32 b) ;

// /
u32x2 operator/(u32x2 a, u32x2 b) ;
u32x2 operator/(u32 a, u32x2 b) ;
u32x2 operator/(u32x2 a, u32 b) ;

// ==
bool operator==(u32x2 a, u32x2 b);
// !=
bool operator!=(u32x2 a, u32x2 b);

// functions
u32 length2(u32x2 a) ;
// u32
// norm(u32x2 a) ;
// u32
// length(u32x2 a) ;
// u32x2
// normalize(u32x2 a) ;
u32 dot(u32x2 a, u32x2 b) ;
u32x2 perpendicular(u32x2 a) ;

//====================
//      u32x3
//====================
// +
u32x3 operator+(u32x3 a, u32x3 b) ;
u32x3 operator+(u32 a, u32x3 b) ;
u32x3 operator+(u32x3 a, u32 b) ;

// -
u32x3 operator-(u32x3 a, u32x3 b) ;
u32x3 operator-(u32 a, u32x3 b) ;
u32x3 operator-(u32x3 a, u32 b) ;
u32x3 operator-(u32x3 a) ;

// *
u32x3 operator*(u32x3 a, u32x3 b) ;
u32x3 operator*(u32 a, u32x3 b) ;
u32x3 operator*(u32x3 a, u32 b) ;

// /
u32x3 operator/(u32x3 a, u32x3 b) ;
u32x3 operator/(u32 a, u32x3 b) ;
u32x3 operator/(u32x3 a, u32 b) ;

// ==
bool operator==(u32x3 a, u32x3 b);
// !=
bool operator!=(u32x3 a, u32x3 b);

// functions
u32 length2(u32x3 a) ;
// u32
// norm(u32x3 a) ;
// u32
// length(u32x3 a) ;
// u32x3
// normalize(u32x3 a) ;
u32 dot(u32x3 a, u32x3 b) ;
u32x3 cross(u32x3 a, u32x3 b) ;


//====================
//      s32x2
//====================
// +
s32x2 operator+(s32x2 a, s32x2 b) ;
s32x2 operator+(s32 a, s32x2 b) ;
s32x2 operator+(s32x2 a, s32 b) ;

// -
s32x2 operator-(s32x2 a, s32x2 b) ;
s32x2 operator-(s32 a, s32x2 b) ;
s32x2 operator-(s32x2 a, s32 b) ;
s32x2 operator-(s32x2 a) ;

// *
s32x2 operator*(s32x2 a, s32x2 b) ;
s32x2 operator*(s32 a, s32x2 b) ;
s32x2 operator*(s32x2 a, s32 b) ;

// /
s32x2 operator/(s32x2 a, s32x2 b) ;
s32x2 operator/(s32 a, s32x2 b) ;
s32x2 operator/(s32x2 a, s32 b) ;

// ==
bool operator==(s32x2 a, s32x2 b);
// !=
bool operator!=(s32x2 a, s32x2 b);

// functions
s32 length2(s32x2 a) ;
// s32
// norm(s32x2 a) ;
// s32
// length(s32x2 a) ;
// s32x2
// normalize(s32x2 a) ;
s32 dot(s32x2 a, s32x2 b) ;
s32x2 perpendicular(s32x2 a) ;

//====================
//      s32x3
//====================
// +
s32x3 operator+(s32x3 a, s32x3 b) ;
s32x3 operator+(s32 a, s32x3 b) ;
s32x3 operator+(s32x3 a, s32 b) ;

// -
s32x3 operator-(s32x3 a, s32x3 b) ;
s32x3 operator-(s32 a, s32x3 b) ;
s32x3 operator-(s32x3 a, s32 b) ;
s32x3 operator-(s32x3 a) ;

// *
s32x3 operator*(s32x3 a, s32x3 b) ;
s32x3 operator*(s32 a, s32x3 b) ;
s32x3 operator*(s32x3 a, s32 b) ;

// /
s32x3 operator/(s32x3 a, s32x3 b) ;
s32x3 operator/(s32 a, s32x3 b) ;
s32x3 operator/(s32x3 a, s32 b) ;

// ==
bool operator==(s32x3 a, s32x3 b);
// !=
bool operator!=(s32x3 a, s32x3 b);

// functions
s32 length2(s32x3 a) ;
// s32
// norm(s32x3 a) ;
// s32
// length(s32x3 a) ;
// s32x3
// normalize(s32x3 a) ;
s32 dot(s32x3 a, s32x3 b) ;
s32x3 cross(s32x3 a, s32x3 b) ;

//====================
//      f32x2
//====================
// +
f32x2 operator+(f32x2 a, f32x2 b) ;
f32x2 operator+(f32 a, f32x2 b) ;
f32x2 operator+(f32x2 a, f32 b) ;

// -
f32x2 operator-(f32x2 a, f32x2 b) ;
f32x2 operator-(f32 a, f32x2 b) ;
f32x2 operator-(f32x2 a, f32 b) ;
f32x2 operator-(f32x2 a) ;

// *
f32x2 operator*(f32x2 a, f32x2 b) ;
f32x2 operator*(f32 a, f32x2 b) ;
f32x2 operator*(f32x2 a, f32 b) ;

// /
f32x2 operator/(f32x2 a, f32x2 b) ;
f32x2 operator/(f32 a, f32x2 b) ;
f32x2 operator/(f32x2 a, f32 b) ;

// ==
bool operator==(f32x2 a, f32x2 b);
// !=
bool operator!=(f32x2 a, f32x2 b);

// functions
f32 length2(f32x2 a) ;
f32 norm(f32x2 a) ;
f32 length(f32x2 a) ;
f32x2 normalize(f32x2 a) ;
f32 dot(f32x2 a, f32x2 b) ;
f32x2 perpendicular(f32x2 a) ;

//====================
//      f32x3
//====================
// +
f32x3 operator+(f32x3 a, f32x3 b) ;
f32x3 operator+(f32 a, f32x3 b) ;
f32x3 operator+(f32x3 a, f32 b) ;

// -
f32x3 operator-(f32x3 a, f32x3 b) ;
f32x3 operator-(f32 a, f32x3 b) ;
f32x3 operator-(f32x3 a, f32 b) ;
f32x3 operator-(f32x3 a) ;

// *
f32x3 operator*(f32x3 a, f32x3 b) ;
f32x3 operator*(f32 a, f32x3 b) ;
f32x3 operator*(f32x3 a, f32 b) ;

// /
f32x3 operator/(f32x3 a, f32x3 b) ;
f32x3 operator/(f32 a, f32x3 b) ;
f32x3 operator/(f32x3 a, f32 b) ;

// ==
bool operator==(f32x3 a, f32x3 b);
// !=
bool operator!=(f32x3 a, f32x3 b);

// functions
f32 length2(f32x3 a) ;
f32 norm(f32x3 a) ;
f32 length(f32x3 a) ;
f32x3 normalize(f32x3 a) ;
f32 dot(f32x3 a, f32x3 b) ;
f32x3 cross(f32x3 a, f32x3 b) ;


// // TODO:
// // f32 ...
// typedef struct {
//     float x, y;
// } V2f32;
//
// typedef struct {
//     float x, y, z;
// } V3f32;
//
// typedef struct {
//     float x, y, z, w;
// } V4f32;
//
// typedef struct {
// } F32;
//
// typedef union {
//     float m[16];
//     float m4[4][4];
//     struct {
//         float m00, m01, m02, m03;
//         float m10, m11, m12, m13;
//         float m20, m21, m22, m23;
//         float m30, m31, m32, m33;
//     };
//     struct {
//         v4f32 row0;
//         v4f32 row1;
//         v4f32 row2;
//         v4f32 row3;
//     }
// } Mat4f32;
//
// typedef struct {
//     /*
//     Quaternion class.
//     A quaternion q = r + i \hat{i} + j \hat{j} + k \hat{k}
//                    = (r,i,j,k)
//     where r is the real part and i,j,k satisfy:
//         i^2=j^2=k^2=ijk=-1
//
//     Quaternions can be used to perform the rotation of 3D vectors:
//         v = (x,y,z)
//         1)
//         -> Create a quaternion: q_v = (0,x,y,z)
//         This quaternion/3d vector can be rotated by angle around the axis:
//         angle = 2*PI/3
//         axis[3] = {1,0,0}
//         q_v_rotated = q_v.rotate(angle, axis[0], axis[1], axis[2])
//
//         2)
//         Create a rotation quaternion and rotate the vector
//         Quaternion = q_rot(angle, axis[0], axis[1], axis[2]);
//         q_rot.convert_to_rotation();
//         q_rot.rotate(v[0], v[1], v[2]);
//
//         3)
//         Quaternion = q_rot(angle, axis[0], axis[1], axis[2]);
//         q_rot.convert_to_rotation();
//         q_v.rotate(q_rot);
//     */
// } Quatf32;


#endif // _LIBMATH_H
