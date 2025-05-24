#ifndef _LIBMATH_H
#define _LIBMATH_H
#include <math.h>
#include "utils/types.h"

//====================
//      s32
//====================
s32
max(s32 a, s32 b) {
    return a > b ? a : b;
}
s32
min(s32 a, s32 b) {
    return a < b ? a : b;
}

//====================
//      f32
//====================
f32
max(f32 a, f32 b) {
    return a > b ? a : b;
}
f32
min(f32 a, f32 b) {
    return a < b ? a : b;
}


//====================
//      s32x2
//====================
// +
s32x2
operator+(s32x2 a, s32x2 b) {
    return {a.x+b.x, a.y+b.y};
}
s32x2
operator+(s32 a, s32x2 b) {
    return {a+b.x, a+b.y};
}
s32x2
operator+(s32x2 a, s32 b) {
    return {a.x+b, a.y+b};
}

// -
s32x2
operator-(s32x2 a, s32x2 b) {
    return {a.x-b.x, a.y-b.y};
}
s32x2
operator-(s32 a, s32x2 b) {
    return {a-b.x, a-b.y};
}
s32x2
operator-(s32x2 a, s32 b) {
    return {a.x-b, a.y-b};
}
s32x2
operator-(s32x2 a) {
    return {-a.x, -a.y};
}

// *
s32x2
operator*(s32x2 a, s32x2 b) {
    return {a.x*b.x, a.y*b.y};
}
s32x2
operator*(s32 a, s32x2 b) {
    return {a*b.x, a*b.y};
}
s32x2
operator*(s32x2 a, s32 b) {
    return {a.x*b, a.y*b};
}

// /
s32x2
operator/(s32x2 a, s32x2 b) {
    return {a.x/b.x, a.y/b.y};
}
s32x2
operator/(s32 a, s32x2 b) {
    return {a/b.x, a/b.y};
}
s32x2
operator/(s32x2 a, s32 b) {
    return {a.x/b, a.y/b};
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
    return {-a.y, a.x};
}

//====================
//      s32x3
//====================
// +
s32x3
operator+(s32x3 a, s32x3 b) {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}
s32x3
operator+(s32 a, s32x3 b) {
    return {a+b.x, a+b.y, a+b.z};
}
s32x3
operator+(s32x3 a, s32 b) {
    return {a.x+b, a.y+b, a.z+b};
}

// -
s32x3
operator-(s32x3 a, s32x3 b) {
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}
s32x3
operator-(s32 a, s32x3 b) {
    return {a-b.x, a-b.y, a-b.z};
}
s32x3
operator-(s32x3 a, s32 b) {
    return {a.x-b, a.y-b, a.z-b};
}
s32x3
operator-(s32x3 a) {
    return {-a.x, -a.y, -a.z};
}

// *
s32x3
operator*(s32x3 a, s32x3 b) {
    return {a.x*b.x, a.y*b.y, a.z*b.z};
}
s32x3
operator*(s32 a, s32x3 b) {
    return {a*b.x, a*b.y, a*b.z};
}
s32x3
operator*(s32x3 a, s32 b) {
    return {a.x*b, a.y*b, a.z*b};
}

// /
s32x3
operator/(s32x3 a, s32x3 b) {
    return {a.x/b.x, a.y/b.y, a.z/b.z};
}
s32x3
operator/(s32 a, s32x3 b) {
    return {a/b.x, a/b.y, a/b.z};
}
s32x3
operator/(s32x3 a, s32 b) {
    return {a.x/b, a.y/b, a.z/b};
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
    return {
        a.y*b.z - a.z*b.y,      \
        a.z*b.x - a.x*b.z,      \
        a.x*b.y - a.y*b.x       \
    };
}

//====================
//      f32x2
//====================
// +
f32x2
operator+(f32x2 a, f32x2 b) {
    return {a.x+b.x, a.y+b.y};
}
f32x2
operator+(f32 a, f32x2 b) {
    return {a+b.x, a+b.y};
}
f32x2
operator+(f32x2 a, f32 b) {
    return {a.x+b, a.y+b};
}

// -
f32x2
operator-(f32x2 a, f32x2 b) {
    return {a.x-b.x, a.y-b.y};
}
f32x2
operator-(f32 a, f32x2 b) {
    return {a-b.x, a-b.y};
}
f32x2
operator-(f32x2 a, f32 b) {
    return {a.x-b, a.y-b};
}
f32x2
operator-(f32x2 a) {
    return {-a.x, -a.y};
}

// *
f32x2
operator*(f32x2 a, f32x2 b) {
    return {a.x*b.x, a.y*b.y};
}
f32x2
operator*(f32 a, f32x2 b) {
    return {a*b.x, a*b.y};
}
f32x2
operator*(f32x2 a, f32 b) {
    return {a.x*b, a.y*b};
}

// /
f32x2
operator/(f32x2 a, f32x2 b) {
    return {a.x/b.x, a.y/b.y};
}
f32x2
operator/(f32 a, f32x2 b) {
    return {a/b.x, a/b.y};
}
f32x2
operator/(f32x2 a, f32 b) {
    return {a.x/b, a.y/b};
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
    return a / length(a);
}
f32
dot(f32x2 a, f32x2 b) {
    return a.x*b.x + a.y*b.y;
}
f32x2
perpendicular(f32x2 a) {
    return {-a.y, a.x};
}

//====================
//      f32x3
//====================
// +
f32x3
operator+(f32x3 a, f32x3 b) {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}
f32x3
operator+(f32 a, f32x3 b) {
    return {a+b.x, a+b.y, a+b.z};
}
f32x3
operator+(f32x3 a, f32 b) {
    return {a.x+b, a.y+b, a.z+b};
}

// -
f32x3
operator-(f32x3 a, f32x3 b) {
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}
f32x3
operator-(f32 a, f32x3 b) {
    return {a-b.x, a-b.y, a-b.z};
}
f32x3
operator-(f32x3 a, f32 b) {
    return {a.x-b, a.y-b, a.z-b};
}
f32x3
operator-(f32x3 a) {
    return {-a.x, -a.y, -a.z};
}

// *
f32x3
operator*(f32x3 a, f32x3 b) {
    return {a.x*b.x, a.y*b.y, a.z*b.z};
}
f32x3
operator*(f32 a, f32x3 b) {
    return {a*b.x, a*b.y, a*b.z};
}
f32x3
operator*(f32x3 a, f32 b) {
    return {a.x*b, a.y*b, a.z*b};
}

// /
f32x3
operator/(f32x3 a, f32x3 b) {
    return {a.x/b.x, a.y/b.y, a.z/b.z};
}
f32x3
operator/(f32 a, f32x3 b) {
    return {a/b.x, a/b.y, a/b.z};
}
f32x3
operator/(f32x3 a, f32 b) {
    return {a.x/b, a.y/b, a.z/b};
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
    return a / length(a);
}
f32
dot(f32x3 a, f32x3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
f32x3
cross(f32x3 a, f32x3 b) {
    return {
        a.y*b.z - a.z*b.y,      \
        a.z*b.x - a.x*b.z,      \
        a.x*b.y - a.y*b.x       \
    };
}

#endif // _LIBMATH_H
