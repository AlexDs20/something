#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "renderer/renderer.h"
#include "libs/ads_images.h"

void print(Vertex* v) {
    printf("Vertex: (%f,%f,%f)\n", v->x, v->y, v->z);
}

void print(VertexAttrs* va) {
    printf("VertexAttrs: uvw=(%f,%f,%f) n=(%f,%f,%f)\n", va->u, va->v, va->w, va->nx, va->ny, va->nz);
}


//==============================
// TODO Temporary!
static f32x3
cross(f32x3 a, f32x3 b) {
    f32x3 result = {
        .x = a.y*b.z - a.z*b.y,      \
        .y = a.z*b.x - a.x*b.z,      \
        .z = a.x*b.y - a.y*b.x       \
    };
    return result;
}
static f32
maxf32(f32 a, f32 b) {
    return a > b ? a : b;
}
static f32
minf32(f32 a, f32 b) {
    return a < b ? a : b;
}
//==============================


static float
absf32(float a) {
    union {
        float f;
        u32 i;
    } u;
    u.f = a;
    u.i &= 0x7fffffff;
    return u.f;
}

void draw_line(u32* framebuffer, u32 w, u32 h, Vertex* a, Vertex* b, u32 c) {
    float dx = b->x-a->x;
    float dy = b->y-a->y;

    u32 steps = absf32(dx)>absf32(dy) ? absf32(dx) : absf32(dy);

    float step_size_x = dx/steps;
    float step_size_y = dy/steps;

    Vertex tmp = *a;
    for (u32 s=0; s<steps; s++) {
        tmp.x += step_size_x;
        tmp.y += step_size_y;

        if (tmp.x<0 || tmp.y<0 || tmp.x>=w || tmp.y>=h) {
            continue;
        }

        u32 linear = (u32)(tmp.y)*w + (u32)tmp.x;
        u32* pixel = framebuffer + linear;
        *pixel = c;
    }
}

/*
 // Supposedly fast from https://haqr.eu/tinyrenderer/bresenham/
 // Need to compare perfs
#include <algorithm>
#include <cmath>
void line(u32* framebuffer, u32 w, u32 h, int ax, int ay, int bx, int by) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int ierror = 0;
    for (int x=ax; x<=bx; x++) {
        if (steep) { // if transposed, de−transpose
            u32 linear = x*w + y;
            u32* pixel = framebuffer + linear;
            *pixel = 0xFFA500;
        } else {
            u32 linear = y*w + x;
            u32* pixel = framebuffer + linear;
            *pixel = 0xFFA500;
        }
        ierror += 2 * std::abs(by-ay);
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx-ax)   * (ierror > bx - ax);
    }
}
*/


void draw_model_wireframe(ObjModel* model, u32 w, u32 h, u32* framebuffer) {
    for (u64 i=0; i<model->n_faces; ++i) {
        ObjFace f = model->faces[i];

        // Only use the x y components atm
        // Can work with perspective and camera later
        Vertex a = *((Vertex*)model->vertices + f.v_indices[0]);
        Vertex b = *((Vertex*)model->vertices + f.v_indices[1]);
        Vertex c = *((Vertex*)model->vertices + f.v_indices[2]);

        // Scale to center of the screen
        a.x *= w;
        a.y *= h;
        b.x *= w;
        b.y *= h;
        c.x *= w;
        c.y *= h;

        draw_line(framebuffer, w, h, &a, &b, 0xFFA500);
        draw_line(framebuffer, w, h, &b, &c, 0xFFA500);
        draw_line(framebuffer, w, h, &c, &a, 0xFFA500);

        // line(framebuffer, w, h, a.x, a.y, b.x, b.y );
        // line(framebuffer, w, h, b.x, b.y, c.x, c.y );
        // line(framebuffer, w, h, c.x, c.y, a.x, a.y );
    }
}

void swap_vertices(Vertex* a, Vertex* b) {
    Vertex tmp = *b;
    *b = *a;
    *a = tmp;
}

void fill_triangle_line_sweep_reference(u32* framebuffer, f32* zbuffer, u32 w, u32 h, Vertex* a, Vertex* b, Vertex* c, u32 color) {
    if (a->y==b->y && a->y==c->y) return; // i dont care about degenerate triangles
    if (a->y>b->y) swap_vertices(a, b);
    if (a->y>c->y) swap_vertices(a, c);
    if (b->y>c->y) swap_vertices(b, c);


    f32 zmid = (a->z + b->z + c->z) / 3;

    s32 ax = (s32)a->x;
    s32 ay = (s32)a->y;
    s32 bx = (s32)b->x;
    s32 by = (s32)b->y;
    s32 cx = (s32)c->x;
    s32 cy = (s32)c->y;

    // static int miny = 100000;
    // static int maxy = -100000;

    int total_height = cy-ay;
    for (int i=0; i<total_height; i++) {
        bool second_half = i>by-ay || by==ay;
        int segment_height = second_half ? cy-by : by-ay;
        float alpha = (float)i/total_height;
        float beta  = (float)(i-(second_half ? by-ay : 0))/segment_height; // be careful: with above conditions no division by zero here
        s32 Ax = ax + (cx-ax)*alpha;
        s32 Ay = ay + (cy-ay)*alpha;
        s32 Bx = second_half ? bx + (cx-bx)*beta : ax + (bx-ax)*beta;
        s32 By = second_half ? by + (cy-by)*beta : ay + (by-ay)*beta;
        if (Ax>Bx) {
            u32 tmp = Ax;
            Ax = Bx;
            Bx = tmp;
            tmp = Ay;
            Ay = By;
            By = tmp;
        }
        for (int j=Ax; j<=Bx; j++) {
            if (ay+i<0 || ay+i>=(s32)h || j<0 || j>=(s32)w) continue;
            // miny = miny < h-(ay+i) ? miny : h-(ay+i);
            // maxy = maxy > h-(ay+i) ? maxy : h-(ay+i);
            u32 linear = w*(h-(ay+i)) + j;

            f32* zpix = zbuffer + linear;
            if (*zpix < zmid) {
                *zpix = zmid;

                u32* pixel = framebuffer + linear;
                *pixel = color;
            }
        }
    }

    // printf("[%d,%d]\n", miny, maxy);
}

f32x3 barycentric_coordinate(f32x2 P, f32x3* A, f32x3* B, f32x3* C) {
    f32x3 v1 = {.x = C->x-A->x, .y = B->x-A->x, .z = A->x-P.x};
    f32x3 v2 = {.x = C->y-A->y, .y = B->y-A->y, .z = A->y-P.y};
    f32x3 u = cross(v1, v2);
    if (absf32(u.z) <= EPS) {
        return {.x = -1, .y = -1, .z = -1};
    }
    return {.x = u.x/u.z, .y = u.y/u.z, .z = 1};
}

typedef union {
    struct {
        f32 left;
        f32 top;
        f32 right;
        f32 bottom;
    };
    f32 data[4];
} Bboxf32;

void fill_triangle_bbox_triangle_check(u32* framebuffer, f32* zbuffer, u32 w, u32 h, Vertex* a, Vertex* b, Vertex* c, u32 color) {
    Bboxf32 bbox = {};
    bbox.left    = minf32(a->x, minf32(b->x, c->x));
    bbox.right   = maxf32(a->x, maxf32(b->x, c->x));
    bbox.bottom  = minf32(a->y, minf32(b->y, c->y));
    bbox.top     = maxf32(a->y, maxf32(b->y, c->y));

    f32 zmid = (a->z + b->z + c->z) / 3;

    for (u32 j=(u32)bbox.bottom; j<=(u32)bbox.top; j++) {
        if (j < 0 || j >= h) continue;
        for (u32 i=(u32)bbox.left; i<=(u32)bbox.right; i++) {
            if (i < 0 || i >= w) continue;
            f32x2 P = {.x=(f32)i, .y=(f32)j};
            f32x3 bary = barycentric_coordinate(P, (f32x3*)a, (f32x3*)b, (f32x3*)c);
            if (bary.x<0 || bary.y<0 || bary.x+bary.y>1) {
                // printf("(%.4f,%.4f,%.4f)\n", bary.x, bary.y, bary.z);
                continue;
            }
            f32* zpix = zbuffer + w*(h-j) + i;
            if (*zpix < zmid) {
                *zpix = zmid;

                u32* pixel = framebuffer + w*(h-j) + i;
                *pixel = color;
            }
        }
    }
}

u32 random_color(u64 v) {
    u32 c = 0xFFFFFF * (v / (f64)RAND_MAX);
    return c;
}

static f32 ceilf32(f32 d) {
    f32 trunc = (f32)((s32)d);
    if (trunc < d) {
        return trunc+1.0f;
    } else {
        return trunc;
    }
}

void shader_frag_color(void* shader_ctx, Vertex* a, Vertex* b, Vertex*c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        f32 w0, f32 w1, f32 w2, u32 x, u32 y, u32 w, u32 h, f32* zbuffer, u32* framebuffer) {
    framebuffer[y*w+x] = ((ColorContext*)shader_ctx)->color;
}

void shader_frag_depth(void* shader_ctx, Vertex* a, Vertex* b, Vertex*c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        f32 w0, f32 w1, f32 w2, u32 x, u32 y, u32 w, u32 h, f32* zbuffer, u32* framebuffer) {
    f32 depth = w0*a->z + w1*b->z + w2*c->z;
    framebuffer[y*w+x] = (u32)(depth*128);
}

void shader_frag_texture(void* shader_ctx, Vertex* a, Vertex* b, Vertex*c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        f32 w0, f32 w1, f32 w2, u32 x, u32 y, u32 width, u32 height, f32* zbuffer, u32* framebuffer) {
    // TextureContext* texture_context = ((TextureContex*)shader_ctx);
    // Check if ARGB ...
    // u32 color = texture_context[texture_x*texture_w + texture_y];
    // u32 color = 0xFF773377;
    f32 u = w0*va->u + w1*vb->u + w2*vc->u;
    f32 v = w0*va->v + w1*vb->v + w2*vc->v;
    f32 w = w0*va->w + w1*vb->w + w2*vc->w;
    // u32 color = (0xFF << 24) | ((u8)(255*u) << 16) | ((u8)(255*v) << 8) | ((u8)(255*w) << 0);

    Image* texture = ((TextureContext*)(shader_ctx))->texture;
    // TODO: NEAREST or BILINEAR INTERP
    //  LoD?
    u32 texture_x = (u32)(u * texture->width);
    u32 texture_y = (u32)(v * texture->height);

    framebuffer[y*width+x] = texture->data[texture_y*texture->width+texture_x];
    // framebuffer[y*width+x] = color;
}

inline f32 compute_triangle_area(f32 ax, f32 ay, f32 bx, f32 by, f32 cx, f32 cy) {
    return 0.5f * ( (by-ay) * (bx+ax) + (cy-by) * (cx+bx) + (ay-cy) * (ax+cx) );
}


void fill_flat_top_triangle(Vertex* a, Vertex* b, Vertex* c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        u32 w, u32 h, u32* framebuffer, f32* zbuffer, void* shader_context, FragmentShader frag_shader) {
    /*
     * assert b->x <= c->x;
     */
    f32 ay = a->y;
    f32 by = b->y;
    f32 cy = c->y;
    f32 ax = a->x;
    f32 bx = b->x;
    f32 cx = c->x;
    f32 az = a->z;
    f32 bz = b->z;
    f32 cz = c->z;

    f32 y_delta_inv = 1.0f / (by - ay);
    f32 ad_inv_slope;
    f32 ae_inv_slope;
    f32 z_left_slope;
    f32 z_right_slope;

    ad_inv_slope = (bx - ax) * y_delta_inv;
    ae_inv_slope = (cx - ax) * y_delta_inv;

    z_left_slope  = (bz - az) * y_delta_inv;
    z_right_slope = (cz - az) * y_delta_inv;

    u32 ys = ay<0 ? 0 : (u32)ceilf32(ay);
    u32 ye = by>h ? h : (u32)ceilf32(by < 0 ? 0 : by);

    f32 xs = ax + (ys - ay) * ad_inv_slope;
    f32 xe = ax + (ys - ay) * ae_inv_slope;

    // z = A_z + (y-A_y) * (B_z-A_z)/(B_y-A_y)
    f32 zs = az + (ys - ay) * z_left_slope;
    f32 ze = az + (ys - ay) * z_right_slope;

    f32 triangle_area = compute_triangle_area(ax, ay, cx, cy, bx, by);

    for (u32 y=ys; y<ye; y++) {
        u32 x_start = xs<0 ? 0 : (u32)ceilf32(xs);
        u32 x_end   = xe>w ? w : (u32)ceilf32( xe<0? 0 : xe );

        f32 z_scanline_slope = 0.0f;
        f32 x_width = (xe-xs);
        if (x_width != 0.0f) {
            z_scanline_slope = (ze-zs)/(xe-xs);
        }
        // z = A_z + (x-A_x) * (B_z-A_z)/(B_x-A_x)
        f32 z = zs + (x_start-xs) * z_scanline_slope;

        u32 offset = w*y;
        for (u32 x=x_start; x<x_end; x++) {
            u32 off = offset + x;
            f32* zpix = zbuffer + off;

            if (*zpix < z) {
                *zpix = z;
                u32* pixel = framebuffer + off;
                // Barycentric:
                f32 alpha = compute_triangle_area(x, y, cx, cy, bx, by) / triangle_area;
                f32 beta = compute_triangle_area(x, y, ax, ay, cx, cy) / triangle_area;
                f32 gamma = compute_triangle_area(x, y, bx, by, ax, ay) / triangle_area;
                // frag_shader(shader_context, w0, w1, w2, x, y, zbuffer, pixel);
                frag_shader(shader_context, a, b, c, va, vb, vc, alpha, beta, gamma, x, y, w, h, zbuffer, framebuffer);
            }
            z += z_scanline_slope;
        }
        xs += ad_inv_slope;
        xe += ae_inv_slope;
        zs += z_left_slope;
        ze += z_right_slope;
    }
}

void fill_flat_bottom_triangle(Vertex* a, Vertex* b, Vertex* c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        u32 w, u32 h, u32* framebuffer, f32* zbuffer, void* shader_context, FragmentShader frag_shader) {
    /*
     * assert a->x <= b->x
     */
    f32 ay = a->y;
    f32 by = b->y;
    f32 cy = c->y;
    f32 ax = a->x;
    f32 bx = b->x;
    f32 cx = c->x;
    f32 az = a->z;
    f32 bz = b->z;
    f32 cz = c->z;

    f32 dc_inv_slope;
    f32 ec_inv_slope;
    f32 z_left_slope;
    f32 z_right_slope;
    f32 dx;
    f32 ex;
    f32 dz;
    f32 ez;

    f32 y_delta_inv = 1.0f / (cy - ay);

    dc_inv_slope = (cx - ax) * y_delta_inv;
    ec_inv_slope = (cx - bx) * y_delta_inv;
    dx = ax;
    ex = bx;
    dz = az;
    ez = bz;

    z_left_slope  = (cz - az) * y_delta_inv;
    z_right_slope = (cz - bz) * y_delta_inv;

    u32 ys = ay<0 ? 0 : (u32)ceilf32(ay);
    u32 ye = cy>h ? h : (u32)ceilf32(cy < 0 ? 0 : cy);

    f32 xs = dx + (ys - ay) * dc_inv_slope;
    f32 xe = ex + (ys - ay) * ec_inv_slope;

    // z = A_z + (y-A_y) * (B_z-A_z)/(B_y-A_y)
    f32 zs = dz + (ys - ay) * z_left_slope;
    f32 ze = ez + (ys - ay) * z_right_slope;

    f32 triangle_area = compute_triangle_area(ax, ay, cx, cy, bx, by);

    for (u32 y=ys; y<ye; y++) {
        u32 x_start = xs<0 ? 0 : (u32)ceilf32(xs);
        u32 x_end   = xe>w ? w : (u32)ceilf32(xe<0 ? 0 : xe);

        f32 z_scanline_slope = 0.0f;
        f32 x_width = (xe-xs);
        if (x_width != 0.0f) {
            z_scanline_slope = (ze-zs)/(xe-xs);
        }
        f32 z = zs + (x_start-xs) * z_scanline_slope;

        u32 offset = w*y;
        for (u32 x=x_start; x<x_end; x++) {
            u32 off = offset + x;
            f32* zpix = zbuffer + off;
            if (*zpix < z) {
                *zpix = z;
                u32* pixel = framebuffer + off;

                // Barycentric
                f32 alpha = compute_triangle_area(x, y, cx, cy, bx, by) / triangle_area;
                f32 beta = compute_triangle_area(x, y, ax, ay, cx, cy) / triangle_area;
                f32 gamma = compute_triangle_area(x, y, bx, by, ax, ay) / triangle_area;
                frag_shader(shader_context, a, b, c, va, vb, vc, alpha, beta, gamma, x, y, w, h, zbuffer, framebuffer);
            }
            z += z_scanline_slope;
        }
        xs += dc_inv_slope;
        xe += ec_inv_slope;
        zs += z_left_slope;
        ze += z_right_slope;
    }
}

void fill_triangle_scanline(u32* framebuffer, f32* zbuffer, u32 w, u32 h,
        Vertex* v1, Vertex* v2, Vertex* v3,
        VertexAttrs* va1, VertexAttrs* va2, VertexAttrs* va3,
        void* shader_context, FragmentShader frag_shader) {
    /*
     * f(t) = A + t * AB            t in [0, 1]
     *
     * x = A_x + t * AB_x
     * y = A_y + t * AB_y
     * z = A_z + t * AB_z
     *
     * IF AB_y != 0 => A_y != B_y
     * x = A_x + ( (y - A_y) / AB_y ) * AB_x
     * x = A_x + (y-A_y) * (AB_x / AB_y)
     *
     * Start at y = A_y
     * until y = B_y
     *
     * y = A_y
     *      x = A_x
     * y = A_y + 1
     *      x = A_x + 1 * (AB_x / AB_y)
     * y = A_y + 2
     *      x = A_x + 2 * (AB_x / AB_y)
     *
     *              C                                 C
     *              x                                 x
     *
     *
     *
     *                  x        ===>           d.        xe        d always on left of e
     *                  B                                 B
     *
     *    x                                 x
     *    A                                 A
     *
     * Z buffer
     * y = A_y + t * AC_y
     * z = A_z + t * AC_z
     *  => z = A_z + ((y-A_y) / (C_y-A_y)) * (C_z - A_z)
     *       = A_z + (y-A_y) * (C_z - A_z)/(C_y-A_y)
     *       = A_z + (x-A_x) * (C_z-A_z)/(C_x-A_x)
     *
     */

    // TODO: Change so that A, B and C are counter clock-wise instead

    Vertex* a = v1;
    Vertex* b = v2;
    Vertex* c = v3;
    VertexAttrs* va = va1;
    VertexAttrs* vb = va2;
    VertexAttrs* vc = va3;

    // Sort in order of ascending y: a.y<b.y<c.y
    Vertex* t;
    VertexAttrs* ta;
    if (b->y < a->y) { t = a; a = b; b = t; ta = va; va = vb; vb = ta;}
    if (c->y < a->y) { t = a; a = c; c = t; ta = va; va = vc; vc = ta;}
    if (c->y < b->y) { t = b; b = c; c = t; ta = vb; vb = vc; vc = ta;}

    // Fast terminate
    if (c->y < 0) return;
    if (a->y >= h) return;
    if (a->y == c->y) return;

    // Fast terminate along x
    f32 xmin = a->x;
    xmin = xmin < b->x ? xmin : b->x;
    xmin = xmin < c->x ? xmin : c->x;
    f32 xmax = a->x;
    xmax = xmax > b->x ? xmax : b->x;
    xmax = xmax > c->x ? xmax : c->x;
    if (xmax < 0) return;
    if (xmin >= w) return;

    // Lower half triangle
    if (b->y == c->y) {
        if (b->x <= c->x) {
            fill_flat_top_triangle(a, b, c, va, vb, vc, w, h, framebuffer, zbuffer, shader_context, frag_shader);
        } else {
            fill_flat_top_triangle(a, c, b, va, vc, vb, w, h, framebuffer, zbuffer, shader_context, frag_shader);
        }
    }
    // Upper half triangle
    else if (a->y == b->y) {
        if (a->x <= b->x) {
            fill_flat_bottom_triangle(a, b, c, va, vb, vc, w, h, framebuffer, zbuffer, shader_context, frag_shader);
        } else {
            fill_flat_bottom_triangle(b, a, c, vb, va, vc, w, h, framebuffer, zbuffer, shader_context, frag_shader);
        }
    }
    else {
        f32 ac_inv_slope = (c->x - a->x) / (c->y - a->y);
        f32 ac_z_inv_slope = (c->z - a->z) / (c->y - a->y);
        f32 extra_x = a->x + (b->y - a->y) * ac_inv_slope;
        f32 extra_z = a->z + (b->y - a->y) * ac_z_inv_slope;

        Vertex extra = {
            .x = extra_x,
            .y = b->y,
            .z = extra_z,
        };

        f32 ac_u_inv_slope = (vc->u - va->u) / (c->y - a->y);
        f32 ac_v_inv_slope = (vc->v - va->v) / (c->y - a->y);
        f32 ac_w_inv_slope = (vc->w - va->w) / (c->y - a->y);
        f32 extra_u = va->u + (b->y - a->y) * ac_u_inv_slope;
        f32 extra_v = va->v + (b->y - a->y) * ac_v_inv_slope;
        f32 extra_w = va->w + (b->y - a->y) * ac_w_inv_slope;

        f32 ac_nx_inv_slope = (vc->nx - va->nx) / (c->y - a->y);
        f32 ac_ny_inv_slope = (vc->ny - va->ny) / (c->y - a->y);
        f32 ac_nz_inv_slope = (vc->nz - va->nz) / (c->y - a->y);
        f32 extra_nx = va->nx + (b->y - a->y) * ac_u_inv_slope;
        f32 extra_ny = va->ny + (b->y - a->y) * ac_v_inv_slope;
        f32 extra_nz = va->nz + (b->y - a->y) * ac_w_inv_slope;

        VertexAttrs ve = {
            .u = extra_u,
            .v = extra_v,
            .w = extra_w,
            .nx = extra_nx,
            .ny = extra_ny,
            .nz = extra_nz,
        };

        if (b->y > 0) {
            if (b->x <= extra.x) {
                fill_flat_top_triangle(a, b, &extra, va, vb, &ve, w, h, framebuffer, zbuffer, shader_context, frag_shader);
            } else {
                fill_flat_top_triangle(a, &extra, b, va, &ve, vb, w, h, framebuffer, zbuffer, shader_context, frag_shader);
            }
        }
        if (b->y < h) {
            if (extra.x <= b->x) {
                fill_flat_bottom_triangle(&extra, b, c, &ve, vb, vc, w, h, framebuffer, zbuffer, shader_context, frag_shader);
            } else {
                fill_flat_bottom_triangle(b, &extra, c, vb, &ve, vc, w, h, framebuffer, zbuffer, shader_context, frag_shader);
            }
        }
    }
}

void draw_model(ObjModel* model, u32 w, u32 h, u32* framebuffer, f32* zbuffer, void* shader_context, FragmentShader frag_shader) {
    for (u64 i=0; i<model->n_faces; ++i) {
        ObjFace* f = model->faces + i;

        // Only use the x y components atm
        // Can work with perspective and camera later
        Vertex a = *(Vertex*)(model->vertices + f->v_indices[0]);
        Vertex b = *(Vertex*)(model->vertices + f->v_indices[1]);
        Vertex c = *(Vertex*)(model->vertices + f->v_indices[2]);

        VertexAttrs va;
        VertexAttrs vb;
        VertexAttrs vc;

        float t;
        t = a.x;
        a.x = (a.z + 0.5f) * w;
        a.y = (a.y + 0.3f) * h;
        a.z = (t+0.05f) * 0.4f;

        t = b.x;
        b.x = (b.z + 0.5f) * w;
        b.y = (b.y + 0.3f) * h;
        b.z = (t+0.05f) * 0.4f;

        t = c.x;
        c.x = (c.z + 0.5f) * w;
        c.y = (c.y + 0.3f) * h;
        c.z = (t+0.05f) * 0.4f;

        fill_triangle_scanline(framebuffer, zbuffer, w, h, &a, &b, &c, &va, &vb, &vc, shader_context, frag_shader);
    }
}
