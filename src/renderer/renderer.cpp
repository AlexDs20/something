#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "renderer/renderer.h"
#include "libs/ads_math.h"

void print(f32x3* v) {
    printf("f32x3: (%f,%f,%f)\n", v->x, v->y, v->z);
}

void draw_line(u32* framebuffer, u32 w, u32 h, f32x3* a, f32x3* b, u32 c) {
    float dx = b->x-a->x;
    float dy = b->y-a->y;

    u32 steps = f32_abs(dx)>f32_abs(dy) ? f32_abs(dx) : f32_abs(dy);

    float step_size_x = dx/steps;
    float step_size_y = dy/steps;

    f32x3 tmp = *a;
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
        f32x3 a = *((f32x3*)model->vertices + f.v_indices[0]);
        f32x3 b = *((f32x3*)model->vertices + f.v_indices[1]);
        f32x3 c = *((f32x3*)model->vertices + f.v_indices[2]);

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

void swap_vertices(f32x3* a, f32x3* b) {
    f32x3 tmp = *b;
    *b = *a;
    *a = tmp;
}

void fill_triangle_line_sweep_reference(u32* framebuffer, f32* zbuffer, u32 w, u32 h, f32x3* a, f32x3* b, f32x3* c, u32 color) {
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

#define EPS (1e-6)
f32x3 barycentric_coordinate(f32x2 P, f32x3* A, f32x3* B, f32x3* C) {
    f32x3 v1 = f32x3_make(C->x-A->x, B->x-A->x, A->x-P.x);
    f32x3 v2 = f32x3_make(C->y-A->y, B->y-A->y, A->y-P.y);
    f32x3 u = f32x3_cross(v1, v2);

    if (f32_abs(u.z) <= EPS) {
        return f32x3_make(-1, -1, -1);
    }
    return f32x3_make(u.x/u.z, u.y/u.z, 1);
}

typedef union {
    struct {
        f32 left;
        f32 top;
        f32 right;
        f32 bottom;
    } edge;
    f32 data[4];
} Bboxf32;

void fill_triangle_bbox_triangle_check(u32* framebuffer, f32* zbuffer, u32 w, u32 h, f32x3* a, f32x3* b, f32x3* c, u32 color) {
    Bboxf32 bbox = {};
    bbox.edge.left    = f32_min(a->x, f32_min(b->x, c->x));
    bbox.edge.right   = f32_max(a->x, f32_max(b->x, c->x));
    bbox.edge.bottom  = f32_min(a->y, f32_min(b->y, c->y));
    bbox.edge.top     = f32_max(a->y, f32_max(b->y, c->y));

    s32 minx = (s32)f32_max(0.0f, f32_floor(bbox.edge.left));
    s32 maxx = (s32)f32_min(w,    f32_ceil(bbox.edge.right));
    s32 miny = (s32)f32_max(0.0f, f32_floor(bbox.edge.bottom));
    s32 maxy = (s32)f32_min(h,    f32_ceil(bbox.edge.top));

    f32 zmid = (a->z + b->z + c->z) / 3;

    for (s32 j=miny; j<maxy; j++) {
        for (s32 i=minx; i<maxx; i++) {
            f32x2 P = f32x2_make((f32)i, (f32)j);
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

#if 0
void shader_frag_color(
        void* shader_ctx,
        f32x3* /*a*/,        f32x3* /*b*/,       f32x3* /*c*/,
        VertexAttrs* /*va*/,  VertexAttrs* /*vb*/, VertexAttrs* /*vc*/,
        f32 /*w0*/,           f32 /*w1*/,          f32 /*w2*/,
        u32 x,            u32 y,           u32 w,           u32 /*h*/,
        f32* /*zbuffer*/,     u32* framebuffer) {
    framebuffer[y*w+x] = ((ColorContext*)shader_ctx)->color;
}

void shader_frag_depth(
        void* /*shader_ctx*/,
        f32x3* a, f32x3* b, f32x3*c,
        VertexAttrs* /*va*/, VertexAttrs* /*vb*/, VertexAttrs* /*vc*/,
        f32 w0, f32 w1, f32 w2,
        u32 x, u32 y, u32 w, u32 /*h*/,
        f32* /*zbuffer*/, u32* framebuffer) {

    f32 depth = w0*a->z + w1*b->z + w2*c->z;
    framebuffer[y*w+x] = (u32)(depth*128);
}
#endif

void shader_frag_color(
        u32* framebuffer , f32* /*zbuffer*/,
        u32 width, u32 /*height*/,
        Material* /*mat*/,
        u32 x, u32 y,
        Vertex* a, Vertex* b, Vertex* /*c*/,
        f32 /*w0*/, f32 /*w1*/, f32 /*w2*/
        ) {

    f32x3 n = f32x3_clamp(255.0f * f32x3_cross(a->position, b->position), f32x3_splat(0.0f), f32x3_splat(255.0f));
    f32 color = ((uint8_t)n.x << 16) | ((uint8_t)n.y << 8) | (uint8_t)n.x;
    framebuffer[y*width+x] = color;

    // framebuffer[y*width+x] = 0xFFA500;
    return;
}

void shader_frag_texture(
        u32* framebuffer , f32* /*zbuffer*/,
        u32 width, u32 /*height*/,
        Material* mat,
        u32 x, u32 y,
        Vertex* a, Vertex* b, Vertex* c,
        f32 w0, f32 w1, f32 w2
        ) {
    // va->u,v,w => are the texture coordinates
    // w0, w1, w2 => barycentric coords

    Texture* texture = mat->map_Kd;
    f32x3* at = &a->texcoords;
    f32x3* bt = &b->texcoords;
    f32x3* ct = &c->texcoords;

    f32 u = w0*at->x + w1*bt->x + w2*ct->x;
    f32 v = w0*at->y + w1*bt->y + w2*ct->y;
    // f32 w = w0*va->w + w1*vb->w + w2*vc->w;

    // TODO: NEAREST or BILINEAR INTERP
    //  LoD?
    u32 texture_x = (u32)(u * texture->width);
    u32 texture_y = (u32)(v * texture->height);

    framebuffer[y*width+x] = texture->data[texture_y*texture->width+texture_x];
    return;
}

inline f32 compute_triangle_area(f32 ax, f32 ay, f32 bx, f32 by, f32 cx, f32 cy) {
    return 0.5f * ( (by-ay) * (bx+ax) + (cy-by) * (cx+bx) + (ay-cy) * (ax+cx) );
}


void fill_flat_top_triangle(u32* framebuffer, f32* zbuffer, u32 w, u32 h, Material* mat, Vertex* v1, Vertex* v2, Vertex* v3) {
    /*
     * assert b->x <= c->x;
     */

    f32x3 a = v1->position;
    f32x3 b = v2->position;
    f32x3 c = v3->position;

    f32 ay = a.y;
    f32 by = b.y;
    f32 cy = c.y;
    f32 ax = a.x;
    f32 bx = b.x;
    f32 cx = c.x;
    f32 az = a.z;
    f32 bz = b.z;
    f32 cz = c.z;

    f32 y_delta_inv = 1.0f / (by - ay);
    f32 ad_inv_slope;
    f32 ae_inv_slope;
    f32 z_left_slope;
    f32 z_right_slope;

    ad_inv_slope = (bx - ax) * y_delta_inv;
    ae_inv_slope = (cx - ax) * y_delta_inv;

    z_left_slope  = (bz - az) * y_delta_inv;
    z_right_slope = (cz - az) * y_delta_inv;

    u32 ys = ay<0 ? 0 : (u32)f32_ceil(ay);
    u32 ye = by>h ? h : (u32)f32_ceil(by < 0 ? 0 : by);

    f32 xs = ax + (ys - ay) * ad_inv_slope;
    f32 xe = ax + (ys - ay) * ae_inv_slope;

    // z = A_z + (y-A_y) * (B_z-A_z)/(B_y-A_y)
    f32 zs = az + (ys - ay) * z_left_slope;
    f32 ze = az + (ys - ay) * z_right_slope;

    f32 triangle_area = compute_triangle_area(ax, ay, cx, cy, bx, by);

    for (u32 y=ys; y<ye; y++) {
        u32 x_start = xs<0 ? 0 : (u32)f32_ceil(xs);
        u32 x_end   = xe>w ? w : (u32)f32_ceil( xe<0? 0 : xe );

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
                // Barycentric:
                f32 alpha = compute_triangle_area(x, y, cx, cy, bx, by) / triangle_area;
                f32 beta = compute_triangle_area(x, y, ax, ay, cx, cy) / triangle_area;
                f32 gamma = compute_triangle_area(x, y, bx, by, ax, ay) / triangle_area;
                // frag_shader(shader_context, w0, w1, w2, x, y, zbuffer, pixel);
                // frag_shader(shader_context, a, b, c, va, vb, vc, alpha, beta, gamma, x, y, w, h, zbuffer, framebuffer);
                shader_frag_texture(framebuffer, zbuffer, w, h, mat, x, y, v1, v2, v3, alpha, beta, gamma);
            }
            z += z_scanline_slope;
        }
        xs += ad_inv_slope;
        xe += ae_inv_slope;
        zs += z_left_slope;
        ze += z_right_slope;
    }
}

void fill_flat_bottom_triangle(u32* framebuffer, f32* zbuffer, u32 w, u32 h, Material* mat, Vertex* v1, Vertex* v2, Vertex* v3) {
    /*
     * assert a->x <= b->x
     */
    f32x3 a = v1->position;
    f32x3 b = v2->position;
    f32x3 c = v3->position;

    f32 ay = a.y;
    f32 by = b.y;
    f32 cy = c.y;
    f32 ax = a.x;
    f32 bx = b.x;
    f32 cx = c.x;
    f32 az = a.z;
    f32 bz = b.z;
    f32 cz = c.z;

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

    u32 ys = ay<0 ? 0 : (u32)f32_ceil(ay);
    u32 ye = cy>h ? h : (u32)f32_ceil(cy < 0 ? 0 : cy);

    f32 xs = dx + (ys - ay) * dc_inv_slope;
    f32 xe = ex + (ys - ay) * ec_inv_slope;

    // z = A_z + (y-A_y) * (B_z-A_z)/(B_y-A_y)
    f32 zs = dz + (ys - ay) * z_left_slope;
    f32 ze = ez + (ys - ay) * z_right_slope;

    f32 triangle_area = compute_triangle_area(ax, ay, cx, cy, bx, by);

    for (u32 y=ys; y<ye; y++) {
        u32 x_start = xs<0 ? 0 : (u32)f32_ceil(xs);
        u32 x_end   = xe>w ? w : (u32)f32_ceil(xe<0 ? 0 : xe);

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
                // Barycentric
                f32 alpha = compute_triangle_area(x, y, cx, cy, bx, by) / triangle_area;
                f32 beta = compute_triangle_area(x, y, ax, ay, cx, cy) / triangle_area;
                f32 gamma = compute_triangle_area(x, y, bx, by, ax, ay) / triangle_area;
                shader_frag_texture(framebuffer, zbuffer, w, h, mat, x, y, v1, v2, v3, alpha, beta, gamma);
            }
            z += z_scanline_slope;
        }
        xs += dc_inv_slope;
        xe += ec_inv_slope;
        zs += z_left_slope;
        ze += z_right_slope;
    }
}

void fill_triangle_scanline(u32* framebuffer, f32* zbuffer, u32 w, u32 h, Material* mat, Vertex* v1, Vertex* v2, Vertex* v3) {
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

    f32x3* a = &v1->position;
    f32x3* b = &v2->position;
    f32x3* c = &v3->position;

    // Sort in order of ascending y: a.y<b.y<c.y
    f32x3* t;
    Vertex* tv;
    if (b->y < a->y) { t = a; a = b; b = t;     tv = v1; v1 = v2; v2 = tv;}
    if (c->y < a->y) { t = a; a = c; c = t;     tv = v1; v1 = v3; v3 = tv;}
    if (c->y < b->y) { t = b; b = c; c = t;     tv = v2; v2 = v3; v3 = tv;}

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
            fill_flat_top_triangle(framebuffer, zbuffer, w, h, mat , v1, v2, v3);
        } else {
            fill_flat_top_triangle(framebuffer, zbuffer, w, h, mat , v1, v3, v2);
        }
    }
    // Upper half triangle
    else if (a->y == b->y) {
        if (a->x <= b->x) {
            fill_flat_bottom_triangle(framebuffer, zbuffer, w, h, mat, v1, v2, v3);
        } else {
            fill_flat_bottom_triangle(framebuffer, zbuffer, w, h, mat, v2, v1, v3);
        }
    }
    else {
        f32 ac_inv_slope = (c->x - a->x) / (c->y - a->y);
        f32 ac_z_inv_slope = (c->z - a->z) / (c->y - a->y);
        f32 extra_x = a->x + (b->y - a->y) * ac_inv_slope;
        f32 extra_z = a->z + (b->y - a->y) * ac_z_inv_slope;

        Vertex extra;
        extra.position.x = extra_x;
        extra.position.y = b->y;
        extra.position.z = extra_z;

        // Handling of the other vertex attributes...
        f32 ac_u_inv_slope = (v3->texcoords.x - v1->texcoords.x) / (v3->position.y - v1->position.y);
        f32 ac_v_inv_slope = (v3->texcoords.y - v1->texcoords.y) / (v3->position.y - v1->position.y);
        f32 ac_w_inv_slope = (v3->texcoords.z - v1->texcoords.z) / (v3->position.y - v1->position.y);
        f32 extra_u = v1->texcoords.x + (v2->position.y - v1->position.y) * ac_u_inv_slope;
        f32 extra_v = v1->texcoords.y + (v2->position.y - v1->position.y) * ac_v_inv_slope;
        f32 extra_w = v1->texcoords.z + (v2->position.y - v1->position.y) * ac_w_inv_slope;

        // TODO: Interpolate the normal as well
        extra.texcoords.x = extra_u;
        extra.texcoords.y = extra_v;
        extra.texcoords.z = extra_w;

        if (b->y > 0) {
            if (b->x <= extra.position.x) {
                fill_flat_top_triangle(framebuffer, zbuffer, w, h, mat, v1, v2, &extra);
            } else {
                fill_flat_top_triangle(framebuffer, zbuffer, w, h, mat, v1, &extra, v2);
            }
        }
        if (b->y < h) {
            if (extra.position.x <= b->x) {
                fill_flat_bottom_triangle(framebuffer, zbuffer, w, h, mat, &extra, v2, v3);
            } else {
                fill_flat_bottom_triangle(framebuffer, zbuffer, w, h, mat, v2, &extra, v3);
            }
        }
    }
}

void draw_submesh(u32* framebuffer, f32* zbuffer, u32 w, u32 h, Material* mat, f32x4x4 transformation, Vertex* vertices, uint32_t* indices, uint32_t n_indices) {
    for (u32 i=0; i<n_indices; i+=3) {
        Vertex* a_ = &vertices[indices[i+0]];
        Vertex* b_ = &vertices[indices[i+1]];
        Vertex* c_ = &vertices[indices[i+2]];

        // vertex shader
        Vertex a, b, c;
        a.position = f32x3_transform_point(&transformation, a_->position);
        a.texcoords = a_->texcoords;
        a.normals   = a_->normals;

        b.position = f32x3_transform_point(&transformation, b_->position);
        b.texcoords = b_->texcoords;
        b.normals   = b_->normals;

        c.position = f32x3_transform_point(&transformation, c_->position);
        c.texcoords = c_->texcoords;
        c.normals   = c_->normals;

        fill_triangle_scanline(framebuffer, zbuffer, w, h, mat, &a, &b, &c);
    }
}

// TODO: Add perspective transformation due to camera
#include "libs/ads_string.h"
void draw_scene(Scene* scene, u32* framebuffer, f32* zbuffer, u32 w, u32 h) {
    f32x4x4 s = f32x4x4_scale_f32(20);
    f32x3 t = {900, 400, -1500};
    Quaternion q_rot = quat_make_rotation({0.0f, 1.0f, 0.0f}, 0);
    f32x4x4 rotation = f32x4x4_from_quat(q_rot);
    f32x4x4 translate = f32x4x4_translate(t);
    f32x4x4 world = translate * rotation * s;

    for (u32 i=0; i<scene->n_objects; i++) {
        Object* obj = &scene->objects[i];

        f32x4x4 transformation = world * obj->transform;

        // if (frustum_cull(obj->bbox)) {}

        Mesh* mesh = obj->mesh;

        uint32_t* ind = mesh->indices;

        for (u32 j=0; j<mesh->n_submeshes; j++) {
            SubMesh* sm = &mesh->submeshes[j];
            uint32_t* sm_indices = &ind[sm->start_index];
            uint32_t ind_count = sm->count;

            if (j == 377) {
                printf("\nSubmesh: %u", j);
                printf(" Material: ");
                sv_print(sm->mat->name);
            }

            draw_submesh(framebuffer, zbuffer, w, h, sm->mat, transformation, mesh->vertices, sm_indices, ind_count);
        }

    }
}
