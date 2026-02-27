#ifndef ADS_MATH_H
#define ADS_MATH_H
#include "utils/types.h"

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

#endif // _LIBMATH_H
