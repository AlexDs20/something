#ifndef ADS_MATH_H
#define ADS_MATH_H
#include <math.h>

#include "base/base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define F32_PI_HALF         1.5707963267948966192f
#define F32_PI              3.1415926535897932384f
#define F32_3_PI_HALF       4.7123889803846898577f
#define F32_TAU             6.2831853071795864769f
#define F32_EPSILON         1e-6f
#define F32_ABS_EPSILON     1e-8f

// bool
static inline bool  bool2_any(bool2 a)                          { return a.x || a.y; }
static inline bool  bool2_all(bool2 a)                          { return a.x && a.y; }
static inline bool2 bool2_not(bool2 a)                          { bool2 r; r.x = !a.x; r.y = !a.y; return r; }
static inline bool2 bool2_and(bool2 a, bool2 b)                 { bool2 r; r.x =  a.x && b.x; r.y = a.y && b.y; return r; }
static inline bool2 bool2_or(bool2 a, bool2 b)                  { bool2 r; r.x =  a.x || b.x; r.y = a.y || b.y; return r; }
static inline bool2 bool2_xor(bool2 a, bool2 b)                 { bool2 r; r.x =  a.x != b.x; r.y = a.y != b.y; return r; }

static inline bool  bool3_any(bool3 a)                          { return a.x || a.y || a.z; }
static inline bool  bool3_all(bool3 a)                          { return a.x && a.y && a.z; }
static inline bool3 bool3_not(bool3 a)                          { bool3 r; r.x = !a.x; r.y = !a.y; r.z = !a.z; return r; }
static inline bool3 bool3_and(bool3 a, bool3 b)                 { bool3 r; r.x =  a.x && b.x; r.y = a.y && b.y; r.z = a.z && b.z; return r; }
static inline bool3 bool3_or(bool3 a, bool3 b)                  { bool3 r; r.x =  a.x || b.x; r.y = a.y || b.y; r.z = a.z || b.z; return r; }
static inline bool3 bool3_xor(bool3 a, bool3 b)                 { bool3 r; r.x =  a.x != b.x; r.y = a.y != b.y; r.z = a.z != b.z; return r; }

static inline bool  bool4_any(bool4 a)                          { return a.x || a.y || a.z || a.w; }
static inline bool  bool4_all(bool4 a)                          { return a.x && a.y && a.z && a.w; }
static inline bool4 bool4_not(bool4 a)                          { bool4 r; r.x = !a.x; r.y = !a.y; r.z = !a.z; r.w = !a.w; return r; }
static inline bool4 bool4_and(bool4 a, bool4 b)                 { bool4 r; r.x =  a.x && b.x; r.y = a.y && b.y; r.z = a.z && b.z; r.w = a.w && b.w; return r; }
static inline bool4 bool4_or(bool4 a, bool4 b)                  { bool4 r; r.x =  a.x || b.x; r.y = a.y || b.y; r.z = a.z || b.z; r.w = a.w || b.w; return r; }
static inline bool4 bool4_xor(bool4 a, bool4 b)                 { bool4 r; r.x =  a.x != b.x; r.y = a.y != b.y; r.z = a.z != b.z; r.w = a.w != b.w; return r; }

// f32
static inline f32   f32_ninf(void)                              { f32Bits t; t.u = 0xFF800000; return t.f; }
static inline f32   f32_inf(void)                               { f32Bits t; t.u = 0x7F800000; return t.f; }
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
static inline f32   f32_sign(f32 a)                             { return (f32)((a > 0.0f) - (a < 0.0f)); }


// f32x2
static inline f32x2 f32x2_make      (f32 x, f32 y)              { f32x2 result; result.x = x; result.y = y; return result; }
static inline f32x2 f32x2_splat     (f32 a)                     { return f32x2_make(a, a); }
static inline f32x2 f32x2_add       (f32x2 a, f32x2 b)          { return f32x2_make(a.x + b.x, a.y + b.y); }
static inline f32x2 f32x2_addl_f32  (f32   a, f32x2 b)          { return f32x2_make(a   + b.x, a   + b.y); }
static inline f32x2 f32x2_addr_f32  (f32x2 a, f32   b)          { return f32x2_make(a.x + b  , a.y + b  ); }
static inline f32x2 f32x2_sub       (f32x2 a, f32x2 b)          { return f32x2_make(a.x - b.x, a.y - b.y); }
static inline f32x2 f32x2_subl_f32  (f32   a, f32x2 b)          { return f32x2_make(a   - b.x, a   - b.y); }
static inline f32x2 f32x2_subr_f32  (f32x2 a, f32   b)          { return f32x2_make(a.x - b  , a.y - b  ); }
static inline f32x2 f32x2_subu      (f32x2 a)                   { return f32x2_make(-a.x     , -a.y     ); }
static inline f32x2 f32x2_mul       (f32x2 a, f32x2 b)          { return f32x2_make(a.x * b.x, a.y * b.y); }
static inline f32x2 f32x2_mull_f32  (f32   a, f32x2 b)          { return f32x2_make(a   * b.x, a   * b.y); }
static inline f32x2 f32x2_mulr_f32  (f32x2 a, f32   b)          { return f32x2_make(a.x * b  , a.y * b  ); }
static inline f32x2 f32x2_div       (f32x2 a, f32x2 b)          { ASSERT(b.x!=0.0f && b.y!=0.0f);      return f32x2_make(a.x / b.x, a.y / b.y); }
static inline f32x2 f32x2_divl_f32  (f32   a, f32x2 b)          { ASSERT(b.x!=0.0f && b.y!=0.0f);      return f32x2_make(a   / b.x, a   / b.y); }
static inline f32x2 f32x2_divr_f32  (f32x2 a, f32   b)          { ASSERT(b!=0.0f); f32 inv = 1.0f / b; return f32x2_make(a.x * inv, a.y * inv); }
static inline bool  f32x2_equal     (f32x2 a, f32x2 b)          { return (   f32_equal(a.x, b.x)  &&   f32_equal(a.y, b.y)  ); }
static inline bool  f32x2_nequal    (f32x2 a, f32x2 b)          { return !f32x2_equal(a, b); }
static inline f32x2 f32x2_min(f32x2 a, f32x2 b)                 { return f32x2_make(f32_min(a.x, b.x), f32_min(a.y, b.y)); }
static inline f32x2 f32x2_max(f32x2 a, f32x2 b)                 { return f32x2_make(f32_max(a.x, b.x), f32_max(a.y, b.y)); }
static inline f32x2 f32x2_clamp(f32x2 x, f32x2 a, f32x2 b)      { return f32x2_min(f32x2_max(x, a), b); }
static inline f32x2 f32x2_floor(f32x2 a)                        { return f32x2_make(f32_floor(a.x), f32_floor(a.y)); }
static inline f32x2 f32x2_ceil(f32x2 a)                         { return f32x2_make(f32_ceil(a.x), f32_ceil(a.y)); }
static inline f32x2 f32x2_abs(f32x2 a)                          { return f32x2_make(f32_abs(a.x), f32_abs(a.y)); }
static inline f32   f32x2_dot(f32x2 a, f32x2 b)                 { return (a.x * b.x + a.y * b.y); }
static inline f32   f32x2_length2(f32x2 a)                      { return (a.x * a.x + a.y * a.y); }
static inline f32   f32x2_length (f32x2 a)                      { return f32_sqrt(f32x2_length2(a)); }
static inline f32x2 f32x2_normalize(f32x2 a)                    { f32 len2 = f32x2_length2(a); ASSERT(len2!=0.0f); f32 inv = f32_rsqrt(len2); return f32x2_make(a.x*inv, a.y*inv); }
static inline f32   f32x2_wedge(f32x2 a, f32x2 b)               { return a.x * b.y - a.y * b.x; }
static inline f32x2 f32x2_perpendicular(f32x2 a)                { return f32x2_make(-a.y, a.x); }
static inline f32x2 f32x2_lerp_fast(f32x2 a, f32x2 b, f32 t)    { return f32x2_make(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
static inline f32x2 f32x2_lerp(f32x2 a, f32x2 b, f32 t)         { return f32x2_make(a.x * (1.0f - t) + b.x * t, a.y * (1.0f - t) + b.y * t); }
// static inline bool  f32x2_interval_overlap(f32x2 a, f32x2 b)    { return (a.y > b.x) && (a.x < b.y); }
static inline f32   f32x2_angle(f32x2 a, f32x2 b)               { return acosf(f32_clamp(f32x2_dot(a, b) * f32_rsqrt(f32x2_length2(a) * f32x2_length2(b)), -1.0f, 1.0f)); }
static inline f32x2 f32x2_mask_select(bool2 mask, f32x2 a, f32x2 b) { f32x2 r; r.x = mask.x ? a.x : b.x; r.y = mask.y ? a.y : b.y; return r; }


// f32x3
static inline f32x3 f32x3_make(f32 x, f32 y, f32 z)             { f32x3 result; result.x = x; result.y = y; result.z = z; return result; }
static inline f32x3 f32x3_splat(f32 a)                          { return f32x3_make(a, a, a); }
static inline f32x3 f32x3_add(f32x3 a, f32x3 b)                 { return f32x3_make(a.x + b.x, a.y + b.y, a.z + b.z); }
static inline f32x3 f32x3_addl_f32(f32   a, f32x3 b)                 { return f32x3_make(a   + b.x, a   + b.y, a   + b.z); }
static inline f32x3 f32x3_addr_f32(f32x3 a, f32   b)                 { return f32x3_make(a.x + b  , a.y + b  , a.z + b  ); }
static inline f32x3 f32x3_sub(f32x3 a, f32x3 b)                 { return f32x3_make(a.x - b.x, a.y - b.y, a.z - b.z); }
static inline f32x3 f32x3_subl_f32(f32   a, f32x3 b)                 { return f32x3_make(a   - b.x, a   - b.y, a   - b.z); }
static inline f32x3 f32x3_subr_f32(f32x3 a, f32   b)                 { return f32x3_make(a.x - b  , a.y - b  , a.z - b  ); }
static inline f32x3 f32x3_subu(f32x3 a)                          { return f32x3_make(-a.x,     -a.y,      -a.z      ); }
static inline f32x3 f32x3_mul(f32x3 a, f32x3 b)                 { return f32x3_make(a.x * b.x, a.y * b.y, a.z * b.z); }
static inline f32x3 f32x3_mull_f32(f32   a, f32x3 b)                 { return f32x3_make(a   * b.x, a   * b.y, a   * b.z); }
static inline f32x3 f32x3_mulr_f32(f32x3 a, f32   b)                 { return f32x3_make(a.x * b  , a.y * b  , a.z * b  ); }
static inline f32x3 f32x3_div(f32x3 a, f32x3 b)                 { ASSERT(b.x!=0.0f && b.y!=0.0f && b.z!=0.0f); return f32x3_make(a.x / b.x, a.y / b.y, a.z / b.z); }
static inline f32x3 f32x3_divl_f32(f32   a, f32x3 b)                 { ASSERT(b.x!=0.0f && b.y!=0.0f && b.z!=0.0f); return f32x3_make(a   / b.x, a   / b.y, a   / b.z); }
static inline f32x3 f32x3_divr_f32(f32x3 a, f32   b)                 { ASSERT(b!=0.0f); f32 inv = 1.0f / b; return f32x3_make(a.x * inv, a.y * inv, a.z * inv); }
static inline bool  f32x3_equal(f32x3 a, f32x3 b)                { return (f32_equal(a.x, b.x) && f32_equal(a.y, b.y) && f32_equal(a.z, b.z)); }
static inline bool  f32x3_nequal(f32x3 a, f32x3 b)                { return !f32x3_equal(a, b); }
static inline f32x3 f32x3_min(f32x3 a, f32x3 b)                 { return f32x3_make(f32_min(a.x, b.x), f32_min(a.y, b.y), f32_min(a.z, b.z)); }
static inline f32x3 f32x3_max(f32x3 a, f32x3 b)                 { return f32x3_make(f32_max(a.x, b.x), f32_max(a.y, b.y), f32_max(a.z, b.z)); }
static inline f32x3 f32x3_clamp(f32x3 x, f32x3 a, f32x3 b)      { return f32x3_min(f32x3_max(x, a), b); }
static inline f32x3 f32x3_floor(f32x3 a)                        { return f32x3_make(f32_floor(a.x), f32_floor(a.y), f32_floor(a.z)); }
static inline f32x3 f32x3_ceil(f32x3 a)                         { return f32x3_make(f32_ceil(a.x), f32_ceil(a.y), f32_ceil(a.z)); }
static inline f32x3 f32x3_abs(f32x3 a)                          { return f32x3_make(f32_abs(a.x), f32_abs(a.y), f32_abs(a.z)); }
static inline f32   f32x3_dot(f32x3 a, f32x3 b)                 { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline f32   f32x3_length2(f32x3 a)                      { return a.x * a.x + a.y * a.y + a.z * a.z; }
static inline f32   f32x3_length (f32x3 a)                      { return f32_sqrt(f32x3_length2(a)); }
static inline f32x3 f32x3_normalize(f32x3 a)                    { f32 len2 = f32x3_length2(a); ASSERT(len2!=0.0f); f32 inv = f32_rsqrt(len2); return f32x3_make(a.x * inv, a.y * inv, a.z * inv); }
static inline f32x3 f32x3_cross(f32x3 a, f32x3 b)               { return f32x3_make(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
static inline f32x3 f32x3_reflect(f32x3 a, f32x3 N)             { ASSERT(f32_equal(f32x3_length2(N), 1.0f)); return f32x3_sub(a, f32x3_mull_f32(2 * f32x3_dot(a, N), N)); }
static inline f32x3 f32x3_project(f32x3 a, f32x3 N)             { ASSERT(f32_equal(f32x3_length2(N), 1.0f)); return f32x3_mull_f32(f32x3_dot(a, N), N); }
static inline f32x3 f32x3_lerp_fast(f32x3 a, f32x3 b, f32 t)    { return f32x3_make(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t); }
static inline f32x3 f32x3_lerp(f32x3 a, f32x3 b, f32 t)         { return f32x3_make(a.x * (1.0f - t) + b.x * t, a.y * (1.0f - t) + b.y * t, a.z * (1.0f - t) + b.z * t); }
static inline f32   f32x3_angle(f32x3 a, f32x3 b)               { return acosf(f32_clamp(f32x3_dot(a, b) * f32_rsqrt(f32x3_length2(a)*f32x3_length2(b)), -1.0f, 1.0f)); }
static inline f32x3 f32x3_refract(f32x3 i, f32x3 N, f32 eta)    { ASSERT(f32_equal(f32x3_length2(N), 1.0f));
     ASSERT(f32_equal(f32x3_length2(i), 1.0f));
     f32 dot_ni = f32x3_dot(N, i);
     f32 k = 1.0f - eta * eta * (1.0f - dot_ni * dot_ni);
     if (k < 0.0f) {
         return f32x3_make(0.0f, 0.0f, 0.0f);
     }
     return f32x3_sub(f32x3_mull_f32(eta, i), f32x3_mull_f32(eta * dot_ni + f32_sqrt(k), N));
}

static inline f32x3 f32x3_transform_point(const f32x4x4* m, f32x3 p) {
    f32x3 result;
    result.x = m->m[0][0] * p.x + m->m[0][1] * p.y + m->m[0][2] * p.z + m->m[0][3];
    result.y = m->m[1][0] * p.x + m->m[1][1] * p.y + m->m[1][2] * p.z + m->m[1][3];
    result.z = m->m[2][0] * p.x + m->m[2][1] * p.y + m->m[2][2] * p.z + m->m[2][3];
    return result;
}
// static inline f32x3 f32x3_transform_vector(const f32x4x4* m, f32x3 p);
static inline f32x3 f32x3_mask_select(bool3 mask, f32x3 a, f32x3 b) { f32x3 r; r.x = mask.y ? a.x : b.x; r.y = mask.y ? a.y : b.y; r.z = mask.z ? a.z : b.z; return r; }


// f32x4
static inline f32x4 f32x4_make(f32 x, f32 y, f32 z, f32 w)      { f32x4 result; result.x = x; result.y = y; result.z = z; result.w = w; return result; }
static inline f32x4 f32x4_splat(f32 a)                          { return f32x4_make(a, a, a, a); }
static inline f32x4 f32x4_add(f32x4 a, f32x4 b)                 { return f32x4_make(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
static inline f32x4 f32x4_addl_f32(f32   a, f32x4 b)            { return f32x4_make(a   + b.x, a   + b.y, a   + b.z, a   + b.w); }
static inline f32x4 f32x4_addr_f32(f32x4 a, f32   b)            { return f32x4_make(a.x + b  , a.y + b  , a.z + b  , a.w + b  ); }
static inline f32x4 f32x4_sub(f32x4 a, f32x4 b)                 { return f32x4_make(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
static inline f32x4 f32x4_subl_f32(f32   a, f32x4 b)            { return f32x4_make(a   - b.x, a   - b.y, a   - b.z, a   - b.w); }
static inline f32x4 f32x4_subr_f32(f32x4 a, f32   b)            { return f32x4_make(a.x - b  , a.y - b  , a.z - b  , a.w - b  ); }
static inline f32x4 f32x4_subu(f32x4 a)                         { return f32x4_make(-a.x, -a.y, -a.z, -a.w); }
static inline f32x4 f32x4_mul(f32x4 a, f32x4 b)                 { return f32x4_make(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
static inline f32x4 f32x4_mull_f32(f32   a, f32x4 b)            { return f32x4_make(a   * b.x, a   * b.y, a   * b.z, a   * b.w); }
static inline f32x4 f32x4_mulr_f32(f32x4 a, f32   b)            { return f32x4_make(a.x * b  , a.y * b  , a.z * b  , a.w * b  ); }
static inline f32x4 f32x4_div(f32x4 a, f32x4 b)                 { ASSERT(b.x!=0.0f && b.y!=0.0f && b.z!=0.0f && b.w!=0.0f); return f32x4_make(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
static inline f32x4 f32x4_divl_f32(f32   a, f32x4 b)            { ASSERT(b.x!=0.0f && b.y!=0.0f && b.z!=0.0f && b.w!=0.0f); return f32x4_make(a   / b.x, a   / b.y, a   / b.z, a   / b.w); }
static inline f32x4 f32x4_divr_f32(f32x4 a, f32   b)            { ASSERT(b!=0.0f); f32 inv = 1.0f / b; return f32x4_make(a.x * inv, a.y * inv, a.z * inv, a.w * inv); }
static inline bool  f32x4_equal(f32x4 a, f32x4 b)               { return (f32_equal(a.x, b.x) && f32_equal(a.y, b.y) && f32_equal(a.z, b.z) && f32_equal(a.w, b.w)); }
static inline bool  f32x4_nequal(f32x4 a, f32x4 b)              { return !f32x4_equal(a, b); }
static inline f32x4 f32x4_min(f32x4 a, f32x4 b)                 { return f32x4_make(f32_min(a.x, b.x), f32_min(a.y, b.y), f32_min(a.z, b.z), f32_min(a.w, b.w)); }
static inline f32x4 f32x4_max(f32x4 a, f32x4 b)                 { return f32x4_make(f32_max(a.x, b.x), f32_max(a.y, b.y), f32_max(a.z, b.z), f32_max(a.w, b.w)); }
static inline f32x4 f32x4_clamp(f32x4 x, f32x4 a, f32x4 b)      { return f32x4_min(f32x4_max(x, a), b); }
static inline f32x4 f32x4_abs(f32x4 a)                          { return f32x4_make(f32_abs(a.x), f32_abs(a.y), f32_abs(a.z), f32_abs(a.w)); }
static inline f32x4 f32x4_floor(f32x4 a)                        { return f32x4_make(f32_floor(a.x), f32_floor(a.y), f32_floor(a.z), f32_floor(a.w)); }
static inline f32x4 f32x4_ceil(f32x4 a)                         { return f32x4_make(f32_ceil(a.x), f32_ceil(a.y), f32_ceil(a.z), f32_ceil(a.w)); }
static inline f32   f32x4_dot(f32x4 a, f32x4 b)                 { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
static inline f32   f32x4_length2(f32x4 a)                      { return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w; }
static inline f32   f32x4_length (f32x4 a)                      { return f32_sqrt(f32x4_length2(a)); }
static inline f32x4 f32x4_normalize(f32x4 a)                    { f32 len2 = f32x4_length2(a); ASSERT(len2!=0.0f); f32 inv = f32_rsqrt(len2); return f32x4_mulr_f32(a, inv); }
static inline f32x4 f32x4_mask_select(bool4 mask, f32x4 a, f32x4 b) { f32x4 r; r.x = mask.x ? a.x : b.x; r.y = mask.y ? a.y : b.y; r.z = mask.z ? a.z : b.z; r.w = mask.w ? a.w : b.w; return r; }

// static inline f32x3 f32x4_perspective_divide(f32x4 a);

// Matrix 4x4
// Row major
static inline f32x4x4 f32x4x4_make(f32x4 row0, f32x4 row1, f32x4 row2, f32x4 row3)  {f32x4x4 r; r.rows.row0=row0; r.rows.row1=row1; r.rows.row2=row2; r.rows.row3=row3; return r;}
static inline f32x4x4 f32x4x4_add(const f32x4x4* a, const f32x4x4* b)           {
    f32x4x4 result;
    result.rows.row0 = f32x4_add(a->rows.row0, b->rows.row0);
    result.rows.row1 = f32x4_add(a->rows.row1, b->rows.row1);
    result.rows.row2 = f32x4_add(a->rows.row2, b->rows.row2);
    result.rows.row3 = f32x4_add(a->rows.row3, b->rows.row3);
    return result;
}
static inline f32x4x4 f32x4x4_sub(const f32x4x4* a, const f32x4x4* b)           {
    f32x4x4 result;
    result.rows.row0 = f32x4_sub(a->rows.row0, b->rows.row0);
    result.rows.row1 = f32x4_sub(a->rows.row1, b->rows.row1);
    result.rows.row2 = f32x4_sub(a->rows.row2, b->rows.row2);
    result.rows.row3 = f32x4_sub(a->rows.row3, b->rows.row3);
    return result;
}

static inline f32x4x4 f32x4x4_mul(const f32x4x4* a, const f32x4x4* b) {
    f32x4x4 result;
    result.m[0][0] = a->m[0][0]*b->m[0][0] + a->m[0][1]*b->m[1][0] + a->m[0][2]*b->m[2][0] + a->m[0][3]*b->m[3][0];
    result.m[0][1] = a->m[0][0]*b->m[0][1] + a->m[0][1]*b->m[1][1] + a->m[0][2]*b->m[2][1] + a->m[0][3]*b->m[3][1];
    result.m[0][2] = a->m[0][0]*b->m[0][2] + a->m[0][1]*b->m[1][2] + a->m[0][2]*b->m[2][2] + a->m[0][3]*b->m[3][2];
    result.m[0][3] = a->m[0][0]*b->m[0][3] + a->m[0][1]*b->m[1][3] + a->m[0][2]*b->m[2][3] + a->m[0][3]*b->m[3][3];

    result.m[1][0] = a->m[1][0]*b->m[0][0] + a->m[1][1]*b->m[1][0] + a->m[1][2]*b->m[2][0] + a->m[1][3]*b->m[3][0];
    result.m[1][1] = a->m[1][0]*b->m[0][1] + a->m[1][1]*b->m[1][1] + a->m[1][2]*b->m[2][1] + a->m[1][3]*b->m[3][1];
    result.m[1][2] = a->m[1][0]*b->m[0][2] + a->m[1][1]*b->m[1][2] + a->m[1][2]*b->m[2][2] + a->m[1][3]*b->m[3][2];
    result.m[1][3] = a->m[1][0]*b->m[0][3] + a->m[1][1]*b->m[1][3] + a->m[1][2]*b->m[2][3] + a->m[1][3]*b->m[3][3];

    result.m[2][0] = a->m[2][0]*b->m[0][0] + a->m[2][1]*b->m[1][0] + a->m[2][2]*b->m[2][0] + a->m[2][3]*b->m[3][0];
    result.m[2][1] = a->m[2][0]*b->m[0][1] + a->m[2][1]*b->m[1][1] + a->m[2][2]*b->m[2][1] + a->m[2][3]*b->m[3][1];
    result.m[2][2] = a->m[2][0]*b->m[0][2] + a->m[2][1]*b->m[1][2] + a->m[2][2]*b->m[2][2] + a->m[2][3]*b->m[3][2];
    result.m[2][3] = a->m[2][0]*b->m[0][3] + a->m[2][1]*b->m[1][3] + a->m[2][2]*b->m[2][3] + a->m[2][3]*b->m[3][3];

    result.m[3][0] = a->m[3][0]*b->m[0][0] + a->m[3][1]*b->m[1][0] + a->m[3][2]*b->m[2][0] + a->m[3][3]*b->m[3][0];
    result.m[3][1] = a->m[3][0]*b->m[0][1] + a->m[3][1]*b->m[1][1] + a->m[3][2]*b->m[2][1] + a->m[3][3]*b->m[3][1];
    result.m[3][2] = a->m[3][0]*b->m[0][2] + a->m[3][1]*b->m[1][2] + a->m[3][2]*b->m[2][2] + a->m[3][3]*b->m[3][2];
    result.m[3][3] = a->m[3][0]*b->m[0][3] + a->m[3][1]*b->m[1][3] + a->m[3][2]*b->m[2][3] + a->m[3][3]*b->m[3][3];
    return result;
}

static inline f32x4 f32x4x4_mul_f32x4(const f32x4x4* a, f32x4   b) {
    f32x4 result;
    result.x = a->m[0][0]*b.x + a->m[0][1]*b.y + a->m[0][2]*b.z + a->m[0][3]*b.w;
    result.y = a->m[1][0]*b.x + a->m[1][1]*b.y + a->m[1][2]*b.z + a->m[1][3]*b.w;
    result.z = a->m[2][0]*b.x + a->m[2][1]*b.y + a->m[2][2]*b.z + a->m[2][3]*b.w;
    result.w = a->m[3][0]*b.x + a->m[3][1]*b.y + a->m[3][2]*b.z + a->m[3][3]*b.w;

    return result;
}
static inline f32x4x4 f32x4x4_transpose(const f32x4x4* a) {
    f32x4x4 result;

    result.m[0][0] = a->m[0][0];
    result.m[0][1] = a->m[1][0];
    result.m[0][2] = a->m[2][0];
    result.m[0][3] = a->m[3][0];

    result.m[1][0] = a->m[0][1];
    result.m[1][1] = a->m[1][1];
    result.m[1][2] = a->m[2][1];
    result.m[1][3] = a->m[3][1];

    result.m[2][0] = a->m[0][2];
    result.m[2][1] = a->m[1][2];
    result.m[2][2] = a->m[2][2];
    result.m[2][3] = a->m[3][2];

    result.m[3][0] = a->m[0][3];
    result.m[3][1] = a->m[1][3];
    result.m[3][2] = a->m[2][3];
    result.m[3][3] = a->m[3][3];

    return result;
}

static inline f32x4x4 f32x4x4_identity(void) {
    f32x4x4 result;
    result.m[0][0] = 1.0f; result.m[0][1] = 0.0f; result.m[0][2] = 0.0f; result.m[0][3] = 0.0f;
    result.m[1][0] = 0.0f; result.m[1][1] = 1.0f; result.m[1][2] = 0.0f; result.m[1][3] = 0.0f;
    result.m[2][0] = 0.0f; result.m[2][1] = 0.0f; result.m[2][2] = 1.0f; result.m[2][3] = 0.0f;
    result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;
    return result;
}

static inline f32x4x4 f32x4x4_translate(f32x3 t) {
    f32x4x4 result;
    result.m[0][0] = 1.0f; result.m[0][1] = 0.0f; result.m[0][2] = 0.0f; result.m[0][3] =  t.x;
    result.m[1][0] = 0.0f; result.m[1][1] = 1.0f; result.m[1][2] = 0.0f; result.m[1][3] =  t.y;
    result.m[2][0] = 0.0f; result.m[2][1] = 0.0f; result.m[2][2] = 1.0f; result.m[2][3] =  t.z;
    result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;
    return result;
}

static inline f32x4x4 f32x4x4_scale_f32(f32 s) {
    f32x4x4 result;
    result.m[0][0] =    s; result.m[0][1] = 0.0f; result.m[0][2] = 0.0f; result.m[0][3] = 0.0f;
    result.m[1][0] = 0.0f; result.m[1][1] =    s; result.m[1][2] = 0.0f; result.m[1][3] = 0.0f;
    result.m[2][0] = 0.0f; result.m[2][1] = 0.0f; result.m[2][2] =    s; result.m[2][3] = 0.0f;
    result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;
    return result;
}

static inline f32x4x4 f32x4x4_scale_f32x3(f32x3 s) {
    f32x4x4 result;
    result.m[0][0] =  s.x; result.m[0][1] = 0.0f; result.m[0][2] = 0.0f; result.m[0][3] = 0.0f;
    result.m[1][0] = 0.0f; result.m[1][1] =  s.y; result.m[1][2] = 0.0f; result.m[1][3] = 0.0f;
    result.m[2][0] = 0.0f; result.m[2][1] = 0.0f; result.m[2][2] =  s.z; result.m[2][3] = 0.0f;
    result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;
    return result;
}

// static inline f32x4x4 f32x4x4_inverse(const f32x4x4& m);
// static inline f32x4x4 f32x4x4_look_at(f32x3 eye, f32x3 target, f32x3 up);
// static inline f32x4x4 f32x4x4_perspective(f32 fov_y_rad, f32 aspect_ratio, f32 near_plane, f32 far_plane);
// static inline f32x4x4 f32x4x4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near_plane, f32 far_plane);

static inline f32x4x4 f32x4x4_from_quat(Quaternion q) {
    f32x4x4 m = f32x4x4_identity();

    f32 xx = q.d.x * q.d.x;
    f32 yy = q.d.y * q.d.y;
    f32 zz = q.d.z * q.d.z;
    f32 xy = q.d.x * q.d.y;
    f32 xz = q.d.x * q.d.z;
    f32 yz = q.d.y * q.d.z;
    f32 wx = q.d.w * q.d.x;
    f32 wy = q.d.w * q.d.y;
    f32 wz = q.d.w * q.d.z;

    m.m[0][0] = 1.0f - 2.0f * (yy + zz);
    m.m[0][1] = 2.0f * (xy - wz);
    m.m[0][2] = 2.0f * (xz + wy);

    m.m[1][0] = 2.0f * (xy + wz);
    m.m[1][1] = 1.0f - 2.0f * (xx + zz);
    m.m[1][2] = 2.0f * (yz - wx);

    m.m[2][0] = 2.0f * (xz - wy);
    m.m[2][1] = 2.0f * (yz + wx);
    m.m[2][2] = 1.0f - 2.0f * (xx + yy);
    return m;
}



// Quaternion
static inline Quaternion quat_make(f32 x, f32 y, f32 z, f32 w)  { Quaternion q; q.d = f32x4_make(x, y, z, w); return q;}
static inline Quaternion quat_make_f32x3(f32x3 a)               { Quaternion q; q.d.x = a.x; q.d.y = a.y; q.d.z = a.z; q.d.w = 0.0f; return q;}
static inline Quaternion quat_identity(void)                    { Quaternion q; q.d = f32x4_make(0.0f,0.0f,0.0f,1.0f); return q; }
static inline Quaternion quat_add(Quaternion a, Quaternion b)  { Quaternion q; q.d = f32x4_add(a.d, b.d); return q; }
static inline Quaternion quat_sub(Quaternion a, Quaternion b)  { Quaternion q; q.d = f32x4_sub(a.d, b.d); return q; }
static inline Quaternion quat_mul(Quaternion a, Quaternion b)  { Quaternion q;
    q.d.x = a.d.w*b.d.x + a.d.x*b.d.w + a.d.y*b.d.z - a.d.z*b.d.y;
    q.d.y = a.d.w*b.d.y - a.d.x*b.d.z + a.d.y*b.d.w + a.d.z*b.d.x;
    q.d.z = a.d.w*b.d.z + a.d.x*b.d.y - a.d.y*b.d.x + a.d.z*b.d.w;
    q.d.w = a.d.w*b.d.w - a.d.x*b.d.x - a.d.y*b.d.y - a.d.z*b.d.z;
    return q;
}
static inline Quaternion quat_mull_f32(f32        a, Quaternion b)  { Quaternion q; q.d = f32x4_mull_f32(a, b.d); return q; }
static inline Quaternion quat_mulr_f32(Quaternion a, f32        b)  { Quaternion q; q.d = f32x4_mulr_f32(a.d, b); return q; }
static inline Quaternion quat_conjugate(Quaternion a)           { Quaternion q = quat_make(-a.d.x, -a.d.y, -a.d.z, a.d.w); return q; }
static inline f32 quat_length2(Quaternion a)                    { return f32x4_length2(a.d); }
static inline f32 quat_length(Quaternion a)                     { return f32x4_length(a.d); }
static inline Quaternion quat_normalize(Quaternion a)           { Quaternion q; q.d = f32x4_normalize(a.d); return q; }
static inline Quaternion quat_reciprocal(Quaternion a)          { return quat_mull_f32(1.0f / quat_length2(a), quat_conjugate(a)); }

static inline Quaternion quat_make_rotation(f32x3 rot_axis, f32 theta) {
    f32 len2 = f32x3_length2(rot_axis);

    ASSERT(f32_equal(len2, 1.0f));
    if (!f32_equal(len2, 1.0f)) {
        rot_axis = f32x3_mull_f32(f32_rsqrt(len2), rot_axis);
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


static inline f32x3 quat_rotate_f32x3(Quaternion q, f32x3 v) {
    // From wiki:
    // r_vec = p_vec + 2*cos(theta/2)sin(theta/2) * (u x p) + 2 * sin2(theta/2) * u x (u x p)
    ASSERT(f32_equal(quat_length2(q), 1.0f));
    f32x3 sin_rot = f32x3_make(q.d.x, q.d.y, q.d.z);
    f32x3 t = f32x3_mull_f32(2.0f, f32x3_cross(sin_rot, v));
    return f32x3_add(v, f32x3_add(f32x3_mull_f32(q.d.w, t), f32x3_cross(sin_rot, t)));
}

// static inline Quaternion quat_slerp(Quaternion a, Quaternion b, f32 t);
// static inline Quaternion quat_make_euler(f32 pitch, f32 yaw, f32 roll);

#ifdef ADS_DEBUG
#include <stdio.h>
#define debug_print_f32(v)                  do { printf("\nf32 %s = %f", #v, (v)); } while(0)
#define debug_print_f32x3(v)                do { printf("\nf32x3 %s = (%f,%f,%f)", #v, (v).x, (v).y, (v).z); } while(0)
#define debug_print_f32x4(v)                do { printf("\nf32x4 %s = (%f,%f,%f,%f)", #v, (v).x, (v).y, (v).z, (v).w); } while(0)
#define debug_print_quat(v)                 do { printf("\nquat %s = (%f,%f,%f,%f)", #v, (v).d.x, (v).d.y, (v).d.z, (v).d.w); } while(0)
#define debug_print_f32x4x4(v)              do { debug_print_f32x4((v).rows.row0); debug_print_f32x4((v).rows.row1); debug_print_f32x4((v).rows.row2); debug_print_f32x4((v).rows.row3);   } while(0)
#else
#define debug_print_f32(x)
#define debug_print_f32x3(v)
#define debug_print_f32x4(v)
#define debug_print_f32x4x4(v)
#endif

#ifdef __cplusplus
}

static inline f32x2 operator+(f32x2 a, f32x2 b)                 { return f32x2_add(a, b);       }
static inline f32x2 operator+(f32   a, f32x2 b)                 { return f32x2_addl_f32(a, b);  }
static inline f32x2 operator+(f32x2 a, f32   b)                 { return f32x2_addr_f32(a, b);  }
static inline f32x2 operator-(f32x2 a, f32x2 b)                 { return f32x2_sub(a, b);       }
static inline f32x2 operator-(f32   a, f32x2 b)                 { return f32x2_subl_f32(a, b);  }
static inline f32x2 operator-(f32x2 a, f32   b)                 { return f32x2_subr_f32(a, b);  }
static inline f32x2 operator-(f32x2 a)                          { return f32x2_subu(a);         }
static inline f32x2 operator*(f32x2 a, f32x2 b)                 { return f32x2_mul(a, b);       }
static inline f32x2 operator*(f32   a, f32x2 b)                 { return f32x2_mull_f32(a, b);  }
static inline f32x2 operator*(f32x2 a, f32   b)                 { return f32x2_mulr_f32(a, b);  }
static inline f32x2 operator/(f32x2 a, f32x2 b)                 { return f32x2_div(a, b);       }
static inline f32x2 operator/(f32   a, f32x2 b)                 { return f32x2_divl_f32(a, b);  }
static inline f32x2 operator/(f32x2 a, f32   b)                 { return f32x2_divr_f32(a, b);  }
static inline bool  operator==(f32x2 a, f32x2 b)                { return f32x2_equal(a, b);     }
static inline bool  operator!=(f32x2 a, f32x2 b)                { return f32x2_nequal(a, b);    }

static inline f32x3 operator+(f32x3 a, f32x3 b)                 { return f32x3_add(a, b);       }
static inline f32x3 operator+(f32   a, f32x3 b)                 { return f32x3_addl_f32(a, b);  }
static inline f32x3 operator+(f32x3 a, f32   b)                 { return f32x3_addr_f32(a, b);  }
static inline f32x3 operator-(f32x3 a, f32x3 b)                 { return f32x3_sub(a, b);       }
static inline f32x3 operator-(f32   a, f32x3 b)                 { return f32x3_subl_f32(a, b);  }
static inline f32x3 operator-(f32x3 a, f32   b)                 { return f32x3_subr_f32(a, b);  }
static inline f32x3 operator-(f32x3 a)                          { return f32x3_subu(a);         }
static inline f32x3 operator*(f32x3 a, f32x3 b)                 { return f32x3_mul(a, b);       }
static inline f32x3 operator*(f32   a, f32x3 b)                 { return f32x3_mull_f32(a, b);  }
static inline f32x3 operator*(f32x3 a, f32   b)                 { return f32x3_mulr_f32(a, b);  }
static inline f32x3 operator/(f32x3 a, f32x3 b)                 { return f32x3_div(a, b);       }
static inline f32x3 operator/(f32   a, f32x3 b)                 { return f32x3_divl_f32(a, b);  }
static inline f32x3 operator/(f32x3 a, f32   b)                 { return f32x3_divr_f32(a, b);  }
static inline bool  operator==(f32x3 a, f32x3 b)                { return f32x3_equal(a, b);     }
static inline bool  operator!=(f32x3 a, f32x3 b)                { return f32x3_nequal(a, b);    }

static inline f32x4 operator+(f32x4 a, f32x4 b)                 { return f32x4_add(a, b);       }
static inline f32x4 operator+(f32   a, f32x4 b)                 { return f32x4_addl_f32(a, b);  }
static inline f32x4 operator+(f32x4 a, f32   b)                 { return f32x4_addr_f32(a, b);  }
static inline f32x4 operator-(f32x4 a, f32x4 b)                 { return f32x4_sub(a, b);       }
static inline f32x4 operator-(f32   a, f32x4 b)                 { return f32x4_subl_f32(a, b);  }
static inline f32x4 operator-(f32x4 a, f32   b)                 { return f32x4_subr_f32(a, b);  }
static inline f32x4 operator-(f32x4 a)                          { return f32x4_subu(a);         }
static inline f32x4 operator*(f32x4 a, f32x4 b)                 { return f32x4_mul(a, b);       }
static inline f32x4 operator*(f32   a, f32x4 b)                 { return f32x4_mull_f32(a, b);  }
static inline f32x4 operator*(f32x4 a, f32   b)                 { return f32x4_mulr_f32(a, b);  }
static inline f32x4 operator/(f32x4 a, f32x4 b)                 { return f32x4_div(a, b);       }
static inline f32x4 operator/(f32   a, f32x4 b)                 { return f32x4_divl_f32(a, b);  }
static inline f32x4 operator/(f32x4 a, f32   b)                 { return f32x4_divr_f32(a, b);  }
static inline bool  operator==(f32x4 a, f32x4 b)                { return f32x4_equal(a, b);     }
static inline bool  operator!=(f32x4 a, f32x4 b)                { return f32x4_nequal(a, b);    }

static inline f32x4x4 operator+(const f32x4x4& a, const f32x4x4& b)     { return f32x4x4_add(&a, &b); }
static inline f32x4x4 operator-(const f32x4x4& a, const f32x4x4& b)     { return f32x4x4_sub(&a, &b); }
static inline f32x4x4 operator*(const f32x4x4& a, const f32x4x4& b)     { return f32x4x4_mul(&a, &b); }
static inline f32x4   operator*(const f32x4x4& a, f32x4   b)            { return f32x4x4_mul_f32x4(&a, b); }

static inline Quaternion operator+(Quaternion a, Quaternion b)          { return quat_add(a, b); }
static inline Quaternion operator-(Quaternion a, Quaternion b)          { return quat_sub(a, b); }
static inline Quaternion operator*(Quaternion a, Quaternion b)          { return quat_mul(a, b); }
static inline Quaternion operator*(f32        a, Quaternion b)          { return quat_mull_f32(a, b); }
static inline Quaternion operator*(Quaternion a, f32        b)          { return quat_mulr_f32(a, b); }

#endif

#endif // ADS_MATH_H
