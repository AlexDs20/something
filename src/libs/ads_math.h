#ifndef ADS_MATH_H
#define ADS_MATH_H
#include <math.h>

#include "base/base.h"

constexpr f32 F32_PI      = 3.1415926535897932384f;
constexpr f32 F32_TAU     = 6.2831853071795864769f;
constexpr f32 F32_EPSILON = 1e-6f;
constexpr f32 F32_ABS_EPSILON = 1e-8f;

// f32
static inline f32   f32_ninf(void)                              { f32Bits t = {.u = 0xFF800000}; return t.f; }
static inline f32   f32_inf(void)                               { f32Bits t = {.u = 0x7F800000}; return t.f; }
static inline f32   f32_min(f32 a, f32 b)                       { return a < b ? a : b; }
static inline f32   f32_max(f32 a, f32 b)                       { return a > b ? a : b; }
static inline f32   f32_clamp(f32 x, f32 a, f32 b)              { return f32_min(f32_max(x, a), b); }
static inline f32   f32_floor(f32 a)                            { return floorf(a); }
static inline f32   f32_ceil(f32 a)                             { return ceilf(a); }
static inline f32   f32_abs(f32 a)                              { return a < 0 ? -a : a; }
static inline bool  f32_equal(f32 a, f32 b)                     { f32 diff = a - b; if (diff < 0) {diff = -diff;} f32 max_val = a > b ? a : b; if (max_val < 0) {max_val = -max_val;} f32 tol = max_val * F32_EPSILON; if (tol < F32_ABS_EPSILON) {tol = F32_ABS_EPSILON;} return diff <= tol; }
static inline f32   f32_sqrt(f32 a)                             { return sqrtf(a); }
static inline f32   f32_rsqrt(f32 a)                            { ASSERT(a>0.0f); return 1.0f / sqrtf(a); }
static inline f32   f32_lerp_fast(f32 a, f32 b, f32 t)          { return a  + (b - a) * t; }
static inline f32   f32_lerp(f32 a, f32 b, f32 t)               { return a * (1.0f - t) + b * t; }
static inline bool  f32_inrange(f32 x, f32 a, f32 b)            { return (x >= a) && (x < b);}
static inline f32   f32_deg_to_rad(f32 d)                       { return d * F32_PI * (1.0f / 180.0f); }
static inline f32   f32_rad_to_deg(f32 r)                       { return r * 180.0f * (1.0f / F32_PI); }
// static inline f32   f32_sign(f32 a)                             { return (a > 0.0f) ? 1.0f : (a < 0.0f ? -1.0f : 0.0f); }
static inline f32   f32_sign(f32 a)                             { return (f32)((a > 0.0f) - (a < 0.0f)); }


// f32x2
static inline f32x2 f32x2_make(f32 x, f32 y)                    { return {x, y}; }
static inline f32x2 f32x2_splat(f32 a)                          { return {a, a}; }
static inline f32x2 operator+(f32x2 a, f32x2 b)                 { return {a.x + b.x, a.y + b.y}; }
static inline f32x2 operator+(f32   a, f32x2 b)                 { return {a   + b.x, a   + b.y}; }
static inline f32x2 operator+(f32x2 a, f32   b)                 { return {a.x + b  , a.y + b  }; }
static inline f32x2 operator-(f32x2 a, f32x2 b)                 { return {a.x - b.x, a.y - b.y}; }
static inline f32x2 operator-(f32   a, f32x2 b)                 { return {a   - b.x, a   - b.y}; }
static inline f32x2 operator-(f32x2 a, f32   b)                 { return {a.x - b  , a.y - b  }; }
static inline f32x2 operator-(f32x2 a)                          { return {-a.x     , -a.y     }; }
static inline f32x2 operator*(f32x2 a, f32x2 b)                 { return {a.x * b.x, a.y * b.y}; }
static inline f32x2 operator*(f32   a, f32x2 b)                 { return {a   * b.x, a   * b.y}; }
static inline f32x2 operator*(f32x2 a, f32   b)                 { return {a.x * b  , a.y * b  }; }
static inline f32x2 operator/(f32x2 a, f32x2 b)                 { ASSERT(b.x!=0.0f && b.y!=0.0f); return {a.x / b.x, a.y / b.y}; }
static inline f32x2 operator/(f32   a, f32x2 b)                 { ASSERT(b.x!=0.0f && b.y!=0.0f); return {a   / b.x, a   / b.y}; }
static inline f32x2 operator/(f32x2 a, f32   b)                 { ASSERT(b!=0.0f); f32 inv = 1.0f / b; return {a.x * inv, a.y * inv}; }
static inline bool  operator==(f32x2 a, f32x2 b)                { return (   f32_equal(a.x, b.x)  &&   f32_equal(a.y, b.y)  ); }
static inline bool  operator!=(f32x2 a, f32x2 b)                { return (!(a==b)); }
static inline f32x2 f32x2_min(f32x2 a, f32x2 b)                 { return {f32_min(a.x, b.x), f32_min(a.y, b.y)}; }
static inline f32x2 f32x2_max(f32x2 a, f32x2 b)                 { return {f32_max(a.x, b.x), f32_max(a.y, b.y)}; }
static inline f32x2 f32x2_clamp(f32x2 x, f32x2 a, f32x2 b)      { return f32x2_min(f32x2_max(x, a), b); }
static inline f32x2 f32x2_floor(f32x2 a)                        { return {f32_floor(a.x), f32_floor(a.y)}; }
static inline f32x2 f32x2_ceil(f32x2 a)                         { return {f32_ceil(a.x), f32_ceil(a.y)}; }
static inline f32x2 f32x2_abs(f32x2 a)                          { return {f32_abs(a.x), f32_abs(a.y)}; }
static inline f32   f32x2_dot(f32x2 a, f32x2 b)                 { return (a.x * b.x + a.y * b.y); }
static inline f32   f32x2_length2(f32x2 a)                      { return (a.x * a.x + a.y * a.y); }
static inline f32   f32x2_length (f32x2 a)                      { return f32_sqrt(f32x2_length2(a)); }
static inline f32x2 f32x2_normalize(f32x2 a)                    { f32 len2 = f32x2_length2(a); ASSERT(len2!=0.0f); f32 inv = f32_rsqrt(len2); return {a.x*inv, a.y*inv}; }
static inline f32   f32x2_wedge(f32x2 a, f32x2 b)               { return a.x * b.y - a.y * b.x; }
static inline f32x2 f32x2_perpendicular(f32x2 a)                { return {-a.y, a.x}; }
static inline f32x2 f32x2_lerp_fast(f32x2 a, f32x2 b, f32 t)    { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t}; }
static inline f32x2 f32x2_lerp(f32x2 a, f32x2 b, f32 t)         { return {a.x * (1.0f - t) + b.x * t, a.y * (1.0f - t) + b.y * t}; }
// static inline bool  f32x2_interval_overlap(f32x2 a, f32x2 b)    { return (a.y > b.x) && (a.x < b.y); }
static inline f32   f32x2_angle(f32x2 a, f32x2 b)               { return acosf(f32_clamp(f32x2_dot(a, b) * f32_rsqrt(f32x2_length2(a) * f32x2_length2(b)), -1.0f, 1.0f)); }


// f32x3
static inline f32x3 f32x3_make(f32 x, f32 y, f32 z)             { return {x, y, z}; }
static inline f32x3 f32x3_splat(f32 a)                          { return {a, a, a}; }
static inline f32x3 operator+(f32x3 a, f32x3 b)                 { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline f32x3 operator+(f32   a, f32x3 b)                 { return {a   + b.x, a   + b.y, a   + b.z}; }
static inline f32x3 operator+(f32x3 a, f32   b)                 { return {a.x + b  , a.y + b  , a.z + b  }; }
static inline f32x3 operator-(f32x3 a, f32x3 b)                 { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline f32x3 operator-(f32   a, f32x3 b)                 { return {a   - b.x, a   - b.y, a   - b.z}; }
static inline f32x3 operator-(f32x3 a, f32   b)                 { return {a.x - b  , a.y - b  , a.z - b  }; }
static inline f32x3 operator-(f32x3 a)                          { return {-a.x,     -a.y,      -a.z      }; }
static inline f32x3 operator*(f32x3 a, f32x3 b)                 { return {a.x * b.x, a.y * b.y, a.z * b.z}; }
static inline f32x3 operator*(f32   a, f32x3 b)                 { return {a   * b.x, a   * b.y, a   * b.z}; }
static inline f32x3 operator*(f32x3 a, f32   b)                 { return {a.x * b  , a.y * b  , a.z * b  }; }
static inline f32x3 operator/(f32x3 a, f32x3 b)                 { ASSERT(b.x!=0.0f && b.y!=0.0f && b.z!=0.0f); return {a.x / b.x, a.y / b.y, a.z / b.z}; }
static inline f32x3 operator/(f32   a, f32x3 b)                 { ASSERT(b.x!=0.0f && b.y!=0.0f && b.z!=0.0f); return {a   / b.x, a   / b.y, a   / b.z}; }
static inline f32x3 operator/(f32x3 a, f32   b)                 { ASSERT(b!=0.0f); f32 inv = 1.0f / b; return {a.x * inv, a.y * inv, a.z * inv}; }
static inline bool  operator==(f32x3 a, f32x3 b)                { return (f32_equal(a.x, b.x) && f32_equal(a.y, b.y) && f32_equal(a.z, b.z)); }
static inline bool  operator!=(f32x3 a, f32x3 b)                { return !(a==b); }
static inline f32x3 f32x3_min(f32x3 a, f32x3 b)                 { return {f32_min(a.x, b.x), f32_min(a.y, b.y), f32_min(a.z, b.z)}; }
static inline f32x3 f32x3_max(f32x3 a, f32x3 b)                 { return {f32_max(a.x, b.x), f32_max(a.y, b.y), f32_max(a.z, b.z)}; }
static inline f32x3 f32x3_clamp(f32x3 x, f32x3 a, f32x3 b)      { return f32x3_min(f32x3_max(x, a), b); }
static inline f32x3 f32x3_floor(f32x3 a)                        { return {f32_floor(a.x), f32_floor(a.y), f32_floor(a.z)}; }
static inline f32x3 f32x3_ceil(f32x3 a)                         { return {f32_ceil(a.x), f32_ceil(a.y), f32_ceil(a.z)}; }
static inline f32x3 f32x3_abs(f32x3 a)                          { return {f32_abs(a.x), f32_abs(a.y), f32_abs(a.z)}; }
static inline f32   f32x3_dot(f32x3 a, f32x3 b)                 { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline f32   f32x3_length2(f32x3 a)                      { return a.x * a.x + a.y * a.y + a.z * a.z; }
static inline f32   f32x3_length (f32x3 a)                      { return f32_sqrt(f32x3_length2(a)); }
static inline f32x3 f32x3_normalize(f32x3 a)                    { f32 len2 = f32x3_length2(a); ASSERT(len2!=0.0f); f32 inv = f32_rsqrt(len2); return {a.x * inv, a.y * inv, a.z * inv}; }
static inline f32x3 f32x3_cross(f32x3 a, f32x3 b)               { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
static inline f32x3 f32x3_reflect(f32x3 a, f32x3 N)             { ASSERT(f32_equal(f32x3_length2(N), 1.0f)); return a - 2 * f32x3_dot(a, N) * N; }
static inline f32x3 f32x3_project(f32x3 a, f32x3 N)             { ASSERT(f32_equal(f32x3_length2(N), 1.0f)); return f32x3_dot(a, N) * N; }
static inline f32x3 f32x3_lerp_fast(f32x3 a, f32x3 b, f32 t)    { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t}; }
static inline f32x3 f32x3_lerp(f32x3 a, f32x3 b, f32 t)         { return {a.x * (1.0f - t) + b.x * t, a.y * (1.0f - t) + b.y * t, a.z * (1.0f - t) + b.z * t}; }
static inline f32   f32x3_angle(f32x3 a, f32x3 b)               { return acosf(f32_clamp(f32x3_dot(a, b) * f32_rsqrt(f32x3_length2(a)*f32x3_length2(b)), -1.0f, 1.0f)); }
static inline f32x3 f32x3_refract(f32x3 i, f32x3 N, f32 eta)    { ASSERT(f32_equal(f32x3_length2(N), 1.0f)); ASSERT(f32_equal(f32x3_length2(i), 1.0f)); f32 dot_ni = f32x3_dot(N, i); f32 k = 1.0f - eta * eta * (1.0f - dot_ni * dot_ni); if (k < 0.0f) { return {0.0f, 0.0f, 0.0f}; } return eta * i - (eta * dot_ni + f32_sqrt(k)) * N; }


// f32x4
static inline f32x4 f32x4_make(f32 x, f32 y, f32 z, f32 w)      { return {x, y, z, w}; }
static inline f32x4 f32x4_splat(f32 a)                          { return {a, a, a, a}; }
static inline f32x4 operator+(f32x4 a, f32x4 b)                 { return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}; }
static inline f32x4 operator+(f32   a, f32x4 b)                 { return {a   + b.x, a   + b.y, a   + b.z, a   + b.w}; }
static inline f32x4 operator+(f32x4 a, f32   b)                 { return {a.x + b  , a.y + b  , a.z + b  , a.w + b  }; }
static inline f32x4 operator-(f32x4 a, f32x4 b)                 { return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}; }
static inline f32x4 operator-(f32   a, f32x4 b)                 { return {a   - b.x, a   - b.y, a   - b.z, a   - b.w}; }
static inline f32x4 operator-(f32x4 a, f32   b)                 { return {a.x - b  , a.y - b  , a.z - b  , a.w - b  }; }
static inline f32x4 operator-(f32x4 a)                          { return {-a.x, -a.y, -a.z, -a.w}; }
static inline f32x4 operator*(f32x4 a, f32x4 b)                 { return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w}; }
static inline f32x4 operator*(f32   a, f32x4 b)                 { return {a   * b.x, a   * b.y, a   * b.z, a   * b.w}; }
static inline f32x4 operator*(f32x4 a, f32   b)                 { return {a.x * b  , a.y * b  , a.z * b  , a.w * b  }; }
static inline f32x4 operator/(f32x4 a, f32x4 b)                 { ASSERT(b.x!=0.0f && b.y!=0.0f && b.z!=0.0f && b.w!=0.0f); return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w}; }
static inline f32x4 operator/(f32   a, f32x4 b)                 { ASSERT(b.x!=0.0f && b.y!=0.0f && b.z!=0.0f && b.w!=0.0f); return {a   / b.x, a   / b.y, a   / b.z, a   / b.w}; }
static inline f32x4 operator/(f32x4 a, f32   b)                 { ASSERT(b!=0.0f); f32 inv = 1.0f / b; return {a.x * inv, a.y * inv, a.z * inv, a.w * inv}; }
static inline bool  operator==(f32x4 a, f32x4 b)                { return (f32_equal(a.x, b.x) && f32_equal(a.y, b.y) && f32_equal(a.z, b.z) && f32_equal(a.w, b.w)); }
static inline bool  operator!=(f32x4 a, f32x4 b)                { return !(a==b); }
static inline f32x4 f32x4_min(f32x4 a, f32x4 b)                 { return {f32_min(a.x, b.x), f32_min(a.y, b.y), f32_min(a.z, b.z), f32_min(a.w, b.w)}; }
static inline f32x4 f32x4_max(f32x4 a, f32x4 b)                 { return {f32_max(a.x, b.x), f32_max(a.y, b.y), f32_max(a.z, b.z), f32_max(a.w, b.w)}; }
static inline f32x4 f32x4_clamp(f32x4 x, f32x4 a, f32x4 b)      { return f32x4_min(f32x4_max(x, a), b); }
static inline f32x4 f32x4_abs(f32x4 a)                          { return {f32_abs(a.x), f32_abs(a.y), f32_abs(a.z), f32_abs(a.w)}; }
static inline f32x4 f32x4_floor(f32x4 a)                        { return {f32_floor(a.x), f32_floor(a.y), f32_floor(a.z), f32_floor(a.w)}; }
static inline f32x4 f32x4_ceil(f32x4 a)                         { return {f32_ceil(a.x), f32_ceil(a.y), f32_ceil(a.z), f32_ceil(a.w)}; }
static inline f32   f32x4_dot(f32x4 a, f32x4 b)                 { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
static inline f32   f32x4_length2(f32x4 a)                      { return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w; }
static inline f32   f32x4_length (f32x4 a)                      { return f32_sqrt(f32x4_length2(a)); }
static inline f32x4 f32x4_normalize(f32x4 a)                    { f32 len2 = f32x4_length2(a); ASSERT(len2!=0.0f); f32 inv = f32_rsqrt(len2); return a * inv; }

// Matrix 4x4
// Row major
static inline f32x4x4 f32x4x4_make(f32x4 row0, f32x4 row1, f32x4 row2, f32x4 row3)  {f32x4x4 r; r.row0=row0; r.row1=row1; r.row2=row2; r.row3=row3; return r;}
static inline f32x4x4 operator+(const f32x4x4& a, const f32x4x4& b)           { f32x4x4 result; result.row0 = a.row0 + b.row0; result.row1 = a.row1 + b.row1; result.row2 = a.row2 + b.row2; result.row3 = a.row3 + b.row3; return result; }
static inline f32x4x4 operator-(const f32x4x4& a, const f32x4x4& b)           { f32x4x4 result; result.row0 = a.row0 - b.row0; result.row1 = a.row1 - b.row1; result.row2 = a.row2 - b.row2; result.row3 = a.row3 - b.row3; return result; }
static inline f32x4x4 operator*(const f32x4x4& a, const f32x4x4& b) {
    f32x4x4 result = {0.0f};
    result.m[0][0] = a.m[0][0]*b.m[0][0] + a.m[0][1]*b.m[1][0] + a.m[0][2]*b.m[2][0] + a.m[0][3]*b.m[3][0];
    result.m[0][1] = a.m[0][0]*b.m[0][1] + a.m[0][1]*b.m[1][1] + a.m[0][2]*b.m[2][1] + a.m[0][3]*b.m[3][1];
    result.m[0][2] = a.m[0][0]*b.m[0][2] + a.m[0][1]*b.m[1][2] + a.m[0][2]*b.m[2][2] + a.m[0][3]*b.m[3][2];
    result.m[0][3] = a.m[0][0]*b.m[0][3] + a.m[0][1]*b.m[1][3] + a.m[0][2]*b.m[2][3] + a.m[0][3]*b.m[3][3];

    result.m[1][0] = a.m[1][0]*b.m[0][0] + a.m[1][1]*b.m[1][0] + a.m[1][2]*b.m[2][0] + a.m[1][3]*b.m[3][0];
    result.m[1][1] = a.m[1][0]*b.m[0][1] + a.m[1][1]*b.m[1][1] + a.m[1][2]*b.m[2][1] + a.m[1][3]*b.m[3][1];
    result.m[1][2] = a.m[1][0]*b.m[0][2] + a.m[1][1]*b.m[1][2] + a.m[1][2]*b.m[2][2] + a.m[1][3]*b.m[3][2];
    result.m[1][3] = a.m[1][0]*b.m[0][3] + a.m[1][1]*b.m[1][3] + a.m[1][2]*b.m[2][3] + a.m[1][3]*b.m[3][3];

    result.m[2][0] = a.m[2][0]*b.m[0][0] + a.m[2][1]*b.m[1][0] + a.m[2][2]*b.m[2][0] + a.m[2][3]*b.m[3][0];
    result.m[2][1] = a.m[2][0]*b.m[0][1] + a.m[2][1]*b.m[1][1] + a.m[2][2]*b.m[2][1] + a.m[2][3]*b.m[3][1];
    result.m[2][2] = a.m[2][0]*b.m[0][2] + a.m[2][1]*b.m[1][2] + a.m[2][2]*b.m[2][2] + a.m[2][3]*b.m[3][2];
    result.m[2][3] = a.m[2][0]*b.m[0][3] + a.m[2][1]*b.m[1][3] + a.m[2][2]*b.m[2][3] + a.m[2][3]*b.m[3][3];

    result.m[3][0] = a.m[3][0]*b.m[0][0] + a.m[3][1]*b.m[1][0] + a.m[3][2]*b.m[2][0] + a.m[3][3]*b.m[3][0];
    result.m[3][1] = a.m[3][0]*b.m[0][1] + a.m[3][1]*b.m[1][1] + a.m[3][2]*b.m[2][1] + a.m[3][3]*b.m[3][1];
    result.m[3][2] = a.m[3][0]*b.m[0][2] + a.m[3][1]*b.m[1][2] + a.m[3][2]*b.m[2][2] + a.m[3][3]*b.m[3][2];
    result.m[3][3] = a.m[3][0]*b.m[0][3] + a.m[3][1]*b.m[1][3] + a.m[3][2]*b.m[2][3] + a.m[3][3]*b.m[3][3];
    return result;
}
static inline f32x4 operator*(const f32x4x4& a, f32x4   b) {
    f32x4 result;

    result.data[0] = a.m[0][0]*b.data[0] + a.m[0][1]*b.data[1] + a.m[0][2]*b.data[2] + a.m[0][3]*b.data[3];
    result.data[1] = a.m[1][0]*b.data[0] + a.m[1][1]*b.data[1] + a.m[1][2]*b.data[2] + a.m[1][3]*b.data[3];
    result.data[2] = a.m[2][0]*b.data[0] + a.m[2][1]*b.data[1] + a.m[2][2]*b.data[2] + a.m[2][3]*b.data[3];
    result.data[3] = a.m[3][0]*b.data[0] + a.m[3][1]*b.data[1] + a.m[3][2]*b.data[2] + a.m[3][3]*b.data[3];

    return result;
}
static inline f32x4x4 f32x4x4_transpose(const f32x4x4& a) {
    f32x4x4 result;

    result.m[0][0] = a.m[0][0];
    result.m[0][1] = a.m[1][0];
    result.m[0][2] = a.m[2][0];
    result.m[0][3] = a.m[3][0];

    result.m[1][0] = a.m[0][1];
    result.m[1][1] = a.m[1][1];
    result.m[1][2] = a.m[2][1];
    result.m[1][3] = a.m[3][1];

    result.m[2][0] = a.m[0][2];
    result.m[2][1] = a.m[1][2];
    result.m[2][2] = a.m[2][2];
    result.m[2][3] = a.m[3][2];

    result.m[3][0] = a.m[0][3];
    result.m[3][1] = a.m[1][3];
    result.m[3][2] = a.m[2][3];
    result.m[3][3] = a.m[3][3];

    return result;
}

// Quaternion
static inline Quaternion quat_make(f32 x, f32 y, f32 z, f32 w)  { Quaternion q; q.d = f32x4_make(x, y, z, w); return q;}
static inline Quaternion quat_make_f32x3(f32x3 a)               { Quaternion q; q.d.x = a.x; q.d.y = a.y; q.d.z = a.z; q.d.w = 0.0f; return q;}
static inline Quaternion quat_identity()                        { Quaternion q; q.d = f32x4_make(0.0f,0.0f,0.0f,1.0f); return q; }
static inline Quaternion operator+(Quaternion a, Quaternion b)  { Quaternion q; q.d = a.d + b.d; return q; }
static inline Quaternion operator-(Quaternion a, Quaternion b)  { Quaternion q; q.d = a.d - b.d; return q; }
static inline Quaternion operator*(Quaternion a, f32        b)  { Quaternion q; q.d = b * a.d; return q; }
static inline Quaternion operator*(f32        a, Quaternion b)  { Quaternion q; q.d = a * b.d; return q; }
static inline Quaternion quat_prod(Quaternion a, Quaternion b)  { Quaternion q;
    q.d.x = a.d.w*b.d.x + a.d.x*b.d.w + a.d.y*b.d.z - a.d.z*b.d.y;
    q.d.y = a.d.w*b.d.y - a.d.x*b.d.z + a.d.y*b.d.w + a.d.z*b.d.x;
    q.d.z = a.d.w*b.d.z + a.d.x*b.d.y - a.d.y*b.d.x + a.d.z*b.d.w;
    q.d.w = a.d.w*b.d.w - a.d.x*b.d.x - a.d.y*b.d.y - a.d.z*b.d.z;
    return q;

}
static inline Quaternion quat_conjugate(Quaternion a)           { Quaternion q = quat_make(-a.d.x, -a.d.y, -a.d.z, a.d.w); return q; }
static inline f32 quat_length2(Quaternion a)                    { return f32x4_length2(a.d); }
static inline f32 quat_length(Quaternion a)                     { return f32x4_length(a.d); }
static inline Quaternion quat_normalize(Quaternion a)           { Quaternion q; q.d = f32x4_normalize(a.d); return q; }
static inline Quaternion quat_reciprocal(Quaternion a)          { return (1.0f / quat_length2(a)) * quat_conjugate(a); }

static inline Quaternion quat_make_rotation(f32x3 rot_axis, f32 theta) {
    f32 len2 = f32x3_length2(rot_axis);

    ASSERT(f32_equal(len2, 1.0f));
    if (!f32_equal(len2, 1.0f)) {
        rot_axis = f32_rsqrt(len2) * rot_axis;
    }

    Quaternion q;
    f32 half_theta = theta * 0.5f;
    f32 st = sinf(half_theta);
    q.d.x  = rot_axis.x * st;
    q.d.y  = rot_axis.y * st;
    q.d.z  = rot_axis.z * st;
    q.d.w  = cosf(half_theta);
    return q;
}

/*
 *
 * static inline f32x3 quat_rotate_f32x3(Quaternion q, f32x3 v) {
 *     ASSERT(f32_equal(quat_length2(q), 1.0f));
 *     f32x3 q_vec = f32x3_make(q.d.x, q.d.y, q.d.z);
 *     f32x3 t = 2.0f * f32x3_cross(q_vec, v);
 *     return v + q.d.w * t + f32x3_cross(q_vec, t);
 * }
 *
 * */


static inline f32x3 quat_rotate_f32x3(Quaternion q, f32x3 v) {
    // Use cross-product instead?
    ASSERT(f32_equal(quat_length2(q), 1.0f));
    // For unit quaternion => inverse is just conjugate
    Quaternion q_inv = quat_conjugate(q);
    Quaternion r = quat_prod(quat_prod(q, quat_make_f32x3(v)), q_inv);
    return f32x3_make(r.d.x, r.d.y, r.d.z);
}

#endif // ADS_MATH_H
