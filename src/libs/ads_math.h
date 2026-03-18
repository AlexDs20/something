#ifndef ADS_MATH_H
#define ADS_MATH_H
#include <math.h>

#include "base/base.h"

#define F32_PI      3.1415926535897932384f
#define F32_TAU     6.2831853071795864769f
#define F32_EPSILON 1e-6f

// f32
static inline f32   f32_min(f32 a, f32 b)                       { return a < b ? a : b; }
static inline f32   f32_max(f32 a, f32 b)                       { return a > b ? a : b; }
static inline f32   f32_clamp(f32 x, f32 a, f32 b)              { return f32_min(f32_max(x, a), b); }
static inline f32   f32_floor(f32 a)                            { return floorf(a); }
static inline f32   f32_ceil(f32 a)                             { return ceilf(a); }
static inline f32   f32_abs(f32 a)                              { return fabsf(a); }
static inline bool  f32_equal(f32 a, f32 b)                     { return f32_abs(a - b) < F32_EPSILON; }            // NOTE: This does not work for large values. Should have "relative" error scaling with values
static inline f32   f32_sqrt(f32 a)                             { return sqrtf(a); }
static inline f32   f32_rsqrt(f32 a)                            { return 1.0f / sqrtf(a); }
static inline f32   f32_lerp_precise(f32 a, f32 b, f32 t)       { return a * (1.0f - t) + b * t; }
static inline f32   f32_lerp_fast(f32 a, f32 b, f32 t)          { return a  + (b - a) * t; }
static inline bool  f32_inrange(f32 x, f32 a, f32 b)            { return (x >= a) && (x < b);}
static inline f32   f32_deg_to_rad(f32 d)                       { return d * F32_PI * (1.0f / 180.0f); }
static inline f32   f32_rad_to_deg(f32 r)                       { return r * 180.0f * (1.0f / F32_PI); }
// static inline f32   f32_sign(f32 a)                             { return (a > 0.0f) ? 1.0f : (a < 0.0f ? -1.0f : 0.0f); }
static inline f32   f32_sign(f32 a)                             { return (f32)((a > 0.0f) - (a < 0.0f)); }


// f32x2
static inline f32x2 f32x2_make(f32 x, f32 y)                    { return {x, y}; }
static inline f32x2 f32x2_splat(f32 a)                          { return {a, a}; }
static inline f32x2 f32x2_min(f32x2 a, f32x2 b)                 { return {f32_min(a.x, b.x), f32_min(a.y, b.y)}; }
static inline f32x2 f32x2_max(f32x2 a, f32x2 b)                 { return {f32_max(a.x, b.x), f32_max(a.y, b.y)}; }
static inline f32x2 f32x2_clamp(f32x2 x, f32 a, f32 b)          { return f32x2_min(f32x2_max(x, a), b); }
static inline f32x2 f32x2_floor(f32x2 a)                        { return {f32_floor(a.x), f32_floor(a.y)}; }
static inline f32x2 f32x2_ceil(f32x2 a)                         { return {f32_ceil(a.x), f32_ceil(a.y)}; }
static inline f32x2 f32x2_abs(f32x2 a)                          { return {f32_abs(a.x), f32_abs(a.y)}; }
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
static inline f32x2 operator/(f32x2 a, f32x2 b)                 { ASSERT(b.x!=0.0f); ASSERT(b.y!=0.0f); return {a.x / b.x, a.y / b.y}; }
static inline f32x2 operator/(f32   a, f32x2 b)                 { ASSERT(b.x!=0.0f); ASSERT(b.y!=0.0f); return {a   / b.x, a   / b.y}; }
static inline f32x2 operator/(f32x2 a, f32   b)                 { ASSERT(b!=0.0f); f32 inv = 1.0f / b; return {a.x * inv, a.y * inv}; }
static inline bool  operator==(f32x2 a, f32x2 b)                { return (   f32_equal(a.x, b.x)  &&   f32_equal(a.y, b.y)  ); }
static inline bool  operator!=(f32x2 a, f32x2 b)                { return ( (!f32_equal(a.x, b.x)) || (!f32_equal(a.y, b.y)) ); }
static inline f32   f32x2_dot(f32x2 a, f32x2 b)                 { return (a.x * b.x + a.y * b.y); }
static inline f32   f32x2_length2(f32x2 a)                      { return (a.x * a.x + a.y * a.y); }
static inline f32   f32x2_length (f32x2 a)                      { return f32_sqrt( a.x * a.x + a.y * a.y ); }
static inline f32x2 f32x2_normalize(f32x2 a)                    { f32 len2 = f32x2_length2(a); ASSERT(len2!=0.0f); if (len2 > F32_EPSILON) { f32 inv = f32_rsqrt(len2); return {a.x*inv, a.y*inv}; } else { return {0.0f, 0.0f}; } }
static inline f32   f32x2_cross(f32x2 a, f32x2 b)               { return a.x * b.y - a.y * b.x; }
static inline f32x2 f32x2_perpendicular(f32x2 a)                { return {-a.y, a.x}; }
static inline f32x2 f32x2_lerp_precise(f32x2 a, f32x2 b, f32 t) { return {a.x * (1.0f - t) + b.x * t, a.y * (1.0f - t) + b.y * t}; }
static inline f32x2 f32x2_lerp_fast(f32x2 a, f32x2 b, f32 t)    { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t}; }
static inline bool  f32x2_interval_overlap(f32x2 a, f32x2 b)    { return (a.y > b.x) && (a.x < b.y); }
static inline f32   f32x2_angle(f32x2 a, f32x2 b)               { return acosf(f32x2_dot(a, b) / (f32x2_length(a) * f32x2_length(b))); }


// f32x3
static inline f32x3 f32x3_make(f32 x, f32 y, f32 z)             { return {x, y, z}; }
static inline f32x3 f32x3_splat(f32 a)                          { return {a, a, a}; }
static inline f32x3 f32x3_min(f32x3 a, f32x3 b)                 { return {f32_min(a.x, b.x), f32_min(a.y, b.y), f32_min(a.z, b.z)}; }
static inline f32x3 f32x3_max(f32x3 a, f32x3 b)                 { return {f32_max(a.x, b.x), f32_max(a.y, b.y), f32_max(a.z, b.z)}; }
static inline f32x3 f32x3_clamp(f32x3 x, f32 a, f32 b)          { return f32x3_min(f32x3_max(x, a), b); }
static inline f32x3 f32x3_floor(f32x3 a)                        { return {f32_floor(a.x), f32_floor(a.y), f32_floor(a.z)}; }
static inline f32x3 f32x3_ceil(f32x3 a)                         { return {f32_ceil(a.x), f32_ceil(a.y), f32_ceil(a.z)}; }
static inline f32x3 f32x3_abs(f32x3 a)                          { return {f32_abs(a.x), f32_abs(a.y), f32_abs(a.z)}; }
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
static inline f32x3 operator/(f32x3 a, f32x3 b)                 { ASSERT(b.x!=0.0f); ASSERT(b.y!=0.0f); ASSERT(b.z!=0.0f); return {a.x / b.x, a.y / b.y, a.z / b.z}; }
static inline f32x3 operator/(f32   a, f32x3 b)                 { ASSERT(b.x!=0.0f); ASSERT(b.y!=0.0f); ASSERT(b.z!=0.0f); return {a   / b.x, a   / b.y, a   / b.z}; }
static inline f32x3 operator/(f32x3 a, f32   b)                 { ASSERT(b!=0.0f); f32 inv = 1.0f / b; return {a.x * inv, a.y * inv, a.z * inv}; }
static inline bool  operator==(f32x3 a, f32x3 b)                { return (f32_equal(a.x, b.x) && f32_equal(a.y, b.y) && f32_equal(a.z, b.z)); }
static inline bool  operator!=(f32x3 a, f32x3 b)                { return !(a==b); }
static inline f32   f32x3_dot(f32x3 a, f32x3 b)                 { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline f32   f32x3_length2(f32x3 a)                      { return a.x * a.x + a.y * a.y + a.z * a.z; }
static inline f32   f32x3_length (f32x3 a)                      { return f32_sqrt(a.x * a.x + a.y * a.y + a.z * a.z); }
static inline f32x3 f32x3_normalize(f32x3 a)                    { f32 len2 = f32x3_length2(a); ASSERT(len2!=0.0f); if (len2 > F32_EPSILON) { f32 inv = f32_rsqrt(len2); return {a.x * inv, a.y * inv, a.z * inv}; } else { return {0.0f, 0.0f, 0.0f}; } }
static inline f32x3 f32x3_cross(f32x3 a, f32x3 b)               { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
static inline f32x3 f32x3_reflect(f32x3 a, f32x3 N)             { ASSERT(f32_equal(f32x3_length2(N), 1.0f)); return a - 2 * f32x3_dot(a, N) * N; }
static inline f32x3 f32x3_project(f32x3 a, f32x3 N)             { ASSERT(f32_equal(f32x3_length2(N), 1.0f)); return f32x3_dot(a, N) * N; }
static inline f32x3 f32x3_lerp(f32x3 a, f32x3 b, f32 t)         { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t}; }
static inline f32   f32x3_angle(f32x3 a, f32x3 b)               { return acosf(f32x3_dot(a, b) / (f32x3_length(a)*f32x3_length(b))); }
static inline f32x3 f32x3_refract(f32x3 a, f32x3 N, f32 eta);


// f32x4
static inline f32x4 f32x4_make(f32 x, f32 y, f32 z, f32 w)      { return {x, y, z, w}; }
static inline f32x4 f32x4_splat(f32 a)                          { return {a, a, a, a}; }
static inline f32x4 f32x4_min(f32x4 a, f32x4 b)                 { return {f32_min(a.x, b.x), f32_min(a.y, b.y), f32_min(a.z, b.z), f32_min(a.w, b.w)}; }
static inline f32x4 f32x4_max(f32x4 a, f32x4 b)                 { return {f32_max(a.x, b.x), f32_max(a.y, b.y), f32_max(a.z, b.z), f32_max(a.w, b.w)}; }
static inline f32x4 f32x4_clamp(f32x4 x, f32 a, f32 b)          { return f32x4_min(f32x4_max(x, a), b); }
static inline f32x4 f32x4_abs(f32x4 a)                          { return {f32_abs(a.x), f32_abs(a.y), f32_abs(a.z), f32_abs(a.w)}; }
static inline f32x4 f32x4_floor(f32x4 a)                        { return {f32_floor(a.x), f32_floor(a.y), f32_floor(a.z), f32_floor(a.w)}; }
static inline f32x4 f32x4_ceil(f32x4 a)                         { return {f32_ceil(a.x), f32_ceil(a.y), f32_ceil(a.z), f32_ceil(a.w)}; }
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
static inline f32x4 operator/(f32x4 a, f32x4 b)                 { ASSERT(b.x!=0.0f); ASSERT(b.y!=0.0f); ASSERT(b.z!=0.0f); ASSERT(b.w!=0.0f); return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w}; }
static inline f32x4 operator/(f32   a, f32x4 b)                 { ASSERT(b.x!=0.0f); ASSERT(b.y!=0.0f); ASSERT(b.z!=0.0f); ASSERT(b.w!=0.0f); return {a   / b.x, a   / b.y, a   / b.z, a   / b.w}; }
static inline f32x4 operator/(f32x4 a, f32   b)                 { ASSERT(b!=0.0f); f32 inv = 1.0f / b; return {a.x * inv, a.y * inv, a.z * inv, a.w * inv}; }
static inline bool  operator==(f32x4 a, f32x4 b)                { return (f32_equal(a.x, b.x) && f32_equal(a.y, b.y) && f32_equal(a.z, b.z) && f32_equal(a.w, b.w)); }
static inline bool  operator!=(f32x4 a, f32x4 b)                { return !(a==b); }
static inline f32   f32x4_dot(f32x4 a, f32x4 b)                 { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
static inline f32   f32x4_length2(f32x4 a)                      { return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w; }
static inline f32   f32x4_length (f32x4 a)                      { return f32_sqrt(f32x4_length2(a)); }
static inline f32x4 f32x4_normalize(f32x4 a)                    { f32 len2 = f32x4_length2(a); ASSERT(len2!=0.0f); if (len2 > F32_EPSILON) { f32 inv = f32_rsqrt(len2); return a * inv; } else { return {0.0f, 0.0f, 0.0f, 0.0f}; } }

// Matrix 4x4

// Quaternion

#endif // ADS_MATH_H_
