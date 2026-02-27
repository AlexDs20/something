#include "ads_math.h"
#include <math.h>
#include "utils/types.h"
//====================
//      u8
//====================
u8
maxu8(u8 a, u8 b) {
    return a > b ? a : b;
}
u8
minu8(u8 a, u8 b) {
    return a < b ? a : b;
}
u8
clampu8(u8 a, u8 low, u8 high) {
    u8 t = a < low ? low : a;
    return t > high ? high : t;
}

//====================
//      s8
//====================
s8
maxs8(s8 a, s8 b) {
    return a > b ? a : b;
}
s8
mins8(s8 a, s8 b) {
    return a < b ? a : b;
}
s8
clamps8(s8 a, s8 low, s8 high) {
    s8 t = a < low ? low : a;
    return t > high ? high : t;
}

//====================
//      u16
//====================
u16
maxu16(u16 a, u16 b) {
    return a > b ? a : b;
}
u16
minu16(u16 a, u16 b) {
    return a < b ? a : b;
}
u16
clampu16(u16 a, u16 low, u16 high) {
    u16 t = a < low ? low : a;
    return t > high ? high : t;
}

//====================
//      s16
//====================
s16
maxs16(s16 a, s8 b) {
    return a > b ? a : b;
}
s16
mins16(s16 a, s8 b) {
    return a < b ? a : b;
}
s16
clamps16(s16 a, s16 low, s16 high) {
    s16 t = a < low ? low : a;
    return t > high ? high : t;
}

//====================
//      u32
//====================
u32
maxu32(u32 a, u32 b) {
    return a > b ? a : b;
}
u32
minu32(u32 a, u32 b) {
    return a < b ? a : b;
}
u32
clampu32(u32 a, u32 low, u32 high) {
    u32 t = a < low ? low : a;
    return t > high ? high : t;
}

//====================
//      s32
//====================
s32
maxs32(s32 a, s32 b) {
    return a > b ? a : b;
}
s32
mins32(s32 a, s32 b) {
    return a < b ? a : b;
}
s32
clamps32(s32 a, s32 low, s32 high) {
    s32 t = a < low ? low : a;
    return t > high ? high : t;
}

//====================
//      f32
//====================
f32
maxf32(f32 a, f32 b) {
    return a > b ? a : b;
}
f32
minf32(f32 a, f32 b) {
    return a < b ? a : b;
}
f32
clampf32(f32 a, f32 low, f32 high) {
    f32 t = a < low ? low : a;
    return t > high ? high : t;
}
/*
u32
ceilf32(f32 a) {
    u32 aint = (u32)a;

    // If the value is its own int
    if (a == (f32)aint) {
        return aint;
    }

    if (a>0) {
        return aint+1;
    }
    return aint;
}
*/

//====================
//      u32x2
//====================
// +
u32x2
operator+(u32x2 a, u32x2 b) {
    u32x2 result = {.x = a.x+b.x, .y = a.y+b.y};
    return result;
}
u32x2
operator+(u32 a, u32x2 b) {
    u32x2 result = {.x = a+b.x, .y = a+b.y};
    return result;
}
u32x2
operator+(u32x2 a, u32 b) {
    u32x2 result = {.x = a.x+b, .y = a.y+b};
    return result;
}

// -
u32x2
operator-(u32x2 a, u32x2 b) {
    u32x2 result = {.x = a.x-b.x, .y = a.y-b.y};
    return result;
}
u32x2
operator-(u32 a, u32x2 b) {
    u32x2 result = {.x = a-b.x, .y = a-b.y};
    return result;
}
u32x2
operator-(u32x2 a, u32 b) {
    u32x2 result = {.x = a.x-b, .y = a.y-b};
    return result;
}

// *
u32x2
operator*(u32x2 a, u32x2 b) {
    u32x2 result = {.x = a.x*b.x, .y = a.y*b.y};
    return result;
}
u32x2
operator*(u32 a, u32x2 b) {
    u32x2 result = {.x = a*b.x, .y = a*b.y};
    return result;
}
u32x2
operator*(u32x2 a, u32 b) {
    u32x2 result = {.x = a.x*b, .y = a.y*b};
    return result;
}

// /
u32x2
operator/(u32x2 a, u32x2 b) {
    u32x2 result = {.x = a.x/b.x, .y = a.y/b.y};
    return result;
}
u32x2
operator/(u32 a, u32x2 b) {
    u32x2 result = {.x = a/b.x, .y = a/b.y};
    return result;
}
u32x2
operator/(u32x2 a, u32 b) {
    u32x2 result = {.x = a.x/b, .y = a.y/b};
    return result;
}

// ==
bool
operator==(u32x2 a, u32x2 b){
    return ((a.x==b.x) && (a.y==b.y));
}
// !=
bool
operator!=(u32x2 a, u32x2 b){
    return ((a.x!=b.x) || (a.y!=b.y));
}

// functions
u32
length2(u32x2 a) {
    return (a.x*a.x + a.y*a.y);
}
// u32
// norm(u32x2 a) {
//     return sqrt(a.x*a.x + a.y*a.y);
// }
// u32
// length(u32x2 a) {
//     return sqrt(a.x*a.x + a.y*a.y);
// }
// u32x2
// normalize(u32x2 a) {
//     return a / length(a);
// }
u32
dot(u32x2 a, u32x2 b) {
    return a.x*b.x + a.y*b.y;
}
u32x2
perpendicular(u32x2 a) {
    u32x2 result = {.x = -a.y, .y = a.x};
    return result;
}

//====================
//      u32x3
//====================
// +
u32x3
operator+(u32x3 a, u32x3 b) {
    u32x3 result = {.x = a.x+b.x, .y = a.y+b.y, .z = a.z+b.z};
    return result;
}
u32x3
operator+(u32 a, u32x3 b) {
    u32x3 result = {.x = a+b.x, .y = a+b.y, .z = a+b.z};
    return result;
}
u32x3
operator+(u32x3 a, u32 b) {
    u32x3 result = {.x = a.x+b, .y = a.y+b, .z = a.z+b};
    return result;
}

// -
u32x3
operator-(u32x3 a, u32x3 b) {
    u32x3 result = {.x = a.x-b.x, .y = a.y-b.y, .z = a.z-b.z};
    return result;
}
u32x3
operator-(u32 a, u32x3 b) {
    u32x3 result = {.x = a-b.x, .y = a-b.y, .z = a-b.z};
    return result;
}
u32x3
operator-(u32x3 a, u32 b) {
    u32x3 result = {.x = a.x-b, .y = a.y-b, .z = a.z-b};
    return result;
}
u32x3
operator-(u32x3 a) {
    u32x3 result = {.x = -a.x, .y = -a.y, .z = -a.z};
    return result;
}

// *
u32x3
operator*(u32x3 a, u32x3 b) {
    u32x3 result = {.x = a.x*b.x, .y = a.y*b.y, .z = a.z*b.z};
    return result;
}
u32x3
operator*(u32 a, u32x3 b) {
    u32x3 result = {.x = a*b.x, .y = a*b.y, .z = a*b.z};
    return result;
}
u32x3
operator*(u32x3 a, u32 b) {
    u32x3 result = {.x = a.x*b, .y = a.y*b, .z = a.z*b};
    return result;
}

// /
u32x3
operator/(u32x3 a, u32x3 b) {
    u32x3 result = {.x = a.x/b.x, .y = a.y/b.y, .z = a.z/b.z};
    return result;
}
u32x3
operator/(u32 a, u32x3 b) {
    u32x3 result = {.x = a/b.x, .y = a/b.y, .z = a/b.z};
    return result;
}
u32x3
operator/(u32x3 a, u32 b) {
    u32x3 result = {.x = a.x/b, .y = a.y/b, .z = a.z/b};
    return result;
}

// ==
bool
operator==(u32x3 a, u32x3 b){
    return ((a.x==b.x) && (a.y==b.y) && (a.z==b.z));
}
// !=
bool
operator!=(u32x3 a, u32x3 b){
    return ((a.x!=b.x) || (a.y!=b.y) || (a.z!=b.z));
}

// functions
u32
length2(u32x3 a) {
    return (a.x*a.x + a.y*a.y + a.z*a.z);
}
// u32
// norm(u32x3 a) {
//     return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
// }
// u32
// length(u32x3 a) {
//     return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
// }
// u32x3
// normalize(u32x3 a) {
//     return a / length(a);
// }
u32
dot(u32x3 a, u32x3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
u32x3
cross(u32x3 a, u32x3 b) {
    u32x3 result = {
        .x = a.y*b.z - a.z*b.y,      \
        .y = a.z*b.x - a.x*b.z,      \
        .z = a.x*b.y - a.y*b.x       \
    };
    return result;
}


//====================
//      s32x2
//====================
// +
s32x2
operator+(s32x2 a, s32x2 b) {
    s32x2 result = {.x = a.x+b.x, .y = a.y+b.y};
    return result;
}
s32x2
operator+(s32 a, s32x2 b) {
    s32x2 result = {.x = a+b.x, .y = a+b.y};
    return result;
}
s32x2
operator+(s32x2 a, s32 b) {
    s32x2 result = {.x = a.x+b, .y = a.y+b};
    return result;
}

// -
s32x2
operator-(s32x2 a, s32x2 b) {
    s32x2 result = {.x = a.x-b.x, .y = a.y-b.y};
    return result;
}
s32x2
operator-(s32 a, s32x2 b) {
    s32x2 result = {.x = a-b.x, .y = a-b.y};
    return result;
}
s32x2
operator-(s32x2 a, s32 b) {
    s32x2 result = {.x = a.x-b, .y = a.y-b};
    return result;
}
s32x2
operator-(s32x2 a) {
    s32x2 result = {.x = -a.x, .y = -a.y};
    return result;
}

// *
s32x2
operator*(s32x2 a, s32x2 b) {
    s32x2 result = {.x = a.x*b.x, .y = a.y*b.y};
    return result;
}
s32x2
operator*(s32 a, s32x2 b) {
    s32x2 result = {.x = a*b.x, .y = a*b.y};
    return result;
}
s32x2
operator*(s32x2 a, s32 b) {
    s32x2 result = {.x = a.x*b, .y = a.y*b};
    return result;
}

// /
s32x2
operator/(s32x2 a, s32x2 b) {
    s32x2 result = {.x = a.x/b.x, .y = a.y/b.y};
    return result;
}
s32x2
operator/(s32 a, s32x2 b) {
    s32x2 result = {.x = a/b.x, .y = a/b.y};
    return result;
}
s32x2
operator/(s32x2 a, s32 b) {
    s32x2 result = {.x = a.x/b, .y = a.y/b};
    return result;
}

// ==
bool
operator==(s32x2 a, s32x2 b){
    return ((a.x==b.x) && (a.y==b.y));
}
// !=
bool
operator!=(s32x2 a, s32x2 b){
    return ((a.x!=b.x) || (a.y!=b.y));
}

// functions
s32
length2(s32x2 a) {
    return (a.x*a.x + a.y*a.y);
}
// s32
// norm(s32x2 a) {
//     return sqrt(a.x*a.x + a.y*a.y);
// }
// s32
// length(s32x2 a) {
//     return sqrt(a.x*a.x + a.y*a.y);
// }
// s32x2
// normalize(s32x2 a) {
//     return a / length(a);
// }
s32
dot(s32x2 a, s32x2 b) {
    return a.x*b.x + a.y*b.y;
}
s32x2
perpendicular(s32x2 a) {
    s32x2 result = {.x = -a.y, .y = a.x};
    return result;
}

//====================
//      s32x3
//====================
// +
s32x3
operator+(s32x3 a, s32x3 b) {
    s32x3 result = {.x = a.x+b.x, .y = a.y+b.y, .z = a.z+b.z};
    return result;
}
s32x3
operator+(s32 a, s32x3 b) {
    s32x3 result = {.x = a+b.x, .y = a+b.y, .z = a+b.z};
    return result;
}
s32x3
operator+(s32x3 a, s32 b) {
    s32x3 result = {.x = a.x+b, .y = a.y+b, .z = a.z+b};
    return result;
}

// -
s32x3
operator-(s32x3 a, s32x3 b) {
    s32x3 result = {.x = a.x-b.x, .y = a.y-b.y, .z = a.z-b.z};
    return result;
}
s32x3
operator-(s32 a, s32x3 b) {
    s32x3 result = {.x = a-b.x, .y = a-b.y, .z = a-b.z};
    return result;
}
s32x3
operator-(s32x3 a, s32 b) {
    s32x3 result = {.x = a.x-b, .y = a.y-b, .z = a.z-b};
    return result;
}
s32x3
operator-(s32x3 a) {
    s32x3 result = {.x = -a.x, .y = -a.y, .z = -a.z};
    return result;
}

// *
s32x3
operator*(s32x3 a, s32x3 b) {
    s32x3 result = {.x = a.x*b.x, .y = a.y*b.y, .z = a.z*b.z};
    return result;
}
s32x3
operator*(s32 a, s32x3 b) {
    s32x3 result = {.x = a*b.x, .y = a*b.y, .z = a*b.z};
    return result;
}
s32x3
operator*(s32x3 a, s32 b) {
    s32x3 result = {.x = a.x*b, .y = a.y*b, .z = a.z*b};
    return result;
}

// /
s32x3
operator/(s32x3 a, s32x3 b) {
    s32x3 result = {.x = a.x/b.x, .y = a.y/b.y, .z = a.z/b.z};
    return result;
}
s32x3
operator/(s32 a, s32x3 b) {
    s32x3 result = {.x = a/b.x, .y = a/b.y, .z = a/b.z};
    return result;
}
s32x3
operator/(s32x3 a, s32 b) {
    s32x3 result = {.x = a.x/b, .y = a.y/b, .z = a.z/b};
    return result;
}

// ==
bool
operator==(s32x3 a, s32x3 b){
    return ((a.x==b.x) && (a.y==b.y) && (a.z==b.z));
}
// !=
bool
operator!=(s32x3 a, s32x3 b){
    return ((a.x!=b.x) || (a.y!=b.y) || (a.z!=b.z));
}

// functions
s32
length2(s32x3 a) {
    return (a.x*a.x + a.y*a.y + a.z*a.z);
}
// s32
// norm(s32x3 a) {
//     return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
// }
// s32
// length(s32x3 a) {
//     return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
// }
// s32x3
// normalize(s32x3 a) {
//     return a / length(a);
// }
s32
dot(s32x3 a, s32x3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
s32x3
cross(s32x3 a, s32x3 b) {
    s32x3 result = {
        .x = a.y*b.z - a.z*b.y,      \
        .y = a.z*b.x - a.x*b.z,      \
        .z = a.x*b.y - a.y*b.x       \
    };
    return result;
}

//====================
//      f32x2
//====================
// +
f32x2
operator+(f32x2 a, f32x2 b) {
    f32x2 result = {.x = a.x+b.x, .y = a.y+b.y};
    return result;
}
f32x2
operator+(f32 a, f32x2 b) {
    f32x2 result = {.x = a+b.x, .y = a+b.y};
    return result;
}
f32x2
operator+(f32x2 a, f32 b) {
    f32x2 result = {.x = a.x+b, .y = a.y+b};
    return result;
}

// -
f32x2
operator-(f32x2 a, f32x2 b) {
    f32x2 result = {.x = a.x-b.x, .y = a.y-b.y};
    return result;
}
f32x2
operator-(f32 a, f32x2 b) {
    f32x2 result = {.x = a-b.x, .y = a-b.y};
    return result;
}
f32x2
operator-(f32x2 a, f32 b) {
    f32x2 result = {.x = a.x-b, .y = a.y-b};
    return result;
}
f32x2
operator-(f32x2 a) {
    f32x2 result = {.x = -a.x, .y = -a.y};
    return result;
}

// *
f32x2
operator*(f32x2 a, f32x2 b) {
    f32x2 result = {.x = a.x*b.x, .y = a.y*b.y};
    return result;
}
f32x2
operator*(f32 a, f32x2 b) {
    f32x2 result = {.x = a*b.x, .y = a*b.y};
    return result;
}
f32x2
operator*(f32x2 a, f32 b) {
    f32x2 result = {.x = a.x*b, .y = a.y*b};
    return result;
}

// /
f32x2
operator/(f32x2 a, f32x2 b) {
    f32x2 result = {.x = a.x/b.x, .y = a.y/b.y};
    return result;
}
f32x2
operator/(f32 a, f32x2 b) {
    f32x2 result = {.x = a/b.x, .y = a/b.y};
    return result;
}
f32x2
operator/(f32x2 a, f32 b) {
    f32x2 result = {.x = a.x/b, .y = a.y/b};
    return result;
}

// ==
bool
operator==(f32x2 a, f32x2 b){
    return ((a.x==b.x) && (a.y==b.y));
}
// !=
bool
operator!=(f32x2 a, f32x2 b){
    return ((a.x!=b.x) || (a.y!=b.y));
}

// functions
f32
length2(f32x2 a) {
    return (a.x*a.x + a.y*a.y);
}
f32
norm(f32x2 a) {
    return sqrt(a.x*a.x + a.y*a.y);
}
f32
length(f32x2 a) {
    return sqrt(a.x*a.x + a.y*a.y);
}
f32x2
normalize(f32x2 a) {
    f32x2 result = a / length(a);
    return result;
}
f32
dot(f32x2 a, f32x2 b) {
    return a.x*b.x + a.y*b.y;
}
f32x2
perpendicular(f32x2 a) {
    f32x2 result = {.x = -a.y, .y = a.x};
    return result;
}

//====================
//      f32x3
//====================
// +
f32x3
operator+(f32x3 a, f32x3 b) {
    f32x3 result = {.x = a.x+b.x, .y = a.y+b.y, .z = a.z+b.z};
    return result;
}
f32x3
operator+(f32 a, f32x3 b) {
    f32x3 result = {.x = a+b.x, .y = a+b.y, .z = a+b.z};
    return result;
}
f32x3
operator+(f32x3 a, f32 b) {
    f32x3 result = {.x = a.x+b, .y = a.y+b, .z = a.z+b};
    return result;
}

// -
f32x3
operator-(f32x3 a, f32x3 b) {
    f32x3 result = {.x = a.x-b.x, .y = a.y-b.y, .z = a.z-b.z};
    return result;
}
f32x3
operator-(f32 a, f32x3 b) {
    f32x3 result = {.x = a-b.x, .y = a-b.y, .z = a-b.z};
    return result;
}
f32x3
operator-(f32x3 a, f32 b) {
    f32x3 result = {.x = a.x-b, .y = a.y-b, .z = a.z-b};
    return result;
}
f32x3
operator-(f32x3 a) {
    f32x3 result = {.x = -a.x, .y = -a.y, .z = -a.z};
    return result;
}

// *
f32x3
operator*(f32x3 a, f32x3 b) {
    f32x3 result = {.x = a.x*b.x, .y = a.y*b.y, .z = a.z*b.z};
    return result;
}
f32x3
operator*(f32 a, f32x3 b) {
    f32x3 result = {.x = a*b.x, .y = a*b.y, .z = a*b.z};
    return result;
}
f32x3
operator*(f32x3 a, f32 b) {
    f32x3 result = {.x = a.x*b, .y = a.y*b, .z = a.z*b};
    return result;
}

// /
f32x3
operator/(f32x3 a, f32x3 b) {
    f32x3 result = {.x = a.x/b.x, .y = a.y/b.y, .z = a.z/b.z};
    return result;
}
f32x3
operator/(f32 a, f32x3 b) {
    f32x3 result = {.x = a/b.x, .y = a/b.y, .z = a/b.z};
    return result;
}
f32x3
operator/(f32x3 a, f32 b) {
    f32x3 result = {.x = a.x/b, .y = a.y/b, .z = a.z/b};
    return result;
}

// ==
bool
operator==(f32x3 a, f32x3 b){
    return ((a.x==b.x) && (a.y==b.y) && (a.z==b.z));
}
// !=
bool
operator!=(f32x3 a, f32x3 b){
    return ((a.x!=b.x) || (a.y!=b.y) || (a.z!=b.z));
}

// functions
f32
length2(f32x3 a) {
    return (a.x*a.x + a.y*a.y + a.z*a.z);
}
f32
norm(f32x3 a) {
    return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}
f32
length(f32x3 a) {
    return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}
f32x3
normalize(f32x3 a) {
    f32x3 result = a / length(a);
    return result;
}
f32
dot(f32x3 a, f32x3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
f32x3
cross(f32x3 a, f32x3 b) {
    f32x3 result = {
        .x = a.y*b.z - a.z*b.y,      \
        .y = a.z*b.x - a.x*b.z,      \
        .z = a.x*b.y - a.y*b.x       \
    };
    return result;
}
