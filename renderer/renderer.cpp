#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "renderer/renderer.h"
#include "libs/ads_math.h"
#include "libs/libstring.h"
#include "libs/ads_images.h"
#include "utils/defines.h"
#include "platform/io.h"

void print(Vertex* v) {
    printf("Vertex: (%f,%f,%f)\n", v->x, v->y, v->z);
}

void print(Normal* n) {
    printf("Normal: (%f,%f,%f)\n", n->x, n->y, n->z);
}

void print(TexCoord* t) {
    printf("TexCoord: (%f,%f,%f)\n", t->u, t->v, t->w);
}

void print(Face* f) {
    printf("Face: vertex: (%d,%d,%d)  texture: (%d,%d,%d)  normal: (%d,%d,%d)\n",
            f->v[0], f->v[1], f->v[2],
            f->vt[0], f->vt[1], f->vt[2],
            f->vn[0], f->vn[1], f->vn[2]
            );
}


f32x3 string_to_f32x3(Arena* arena, string8 data) {
    f32x3 out = {0};
    u64 space = string_find_first(data, ' ');
    string8 r = string_substring(data, 0, space);
    out.r = atof(string_to_cstr(arena, r));

    string_move_forward(&data, space+1);
    space = string_find_first(data, ' ');
    string8 g = string_substring(data, 0, space);
    out.g = atof(string_to_cstr(arena, g));

    string_move_forward(&data, space+1);
    space = string_find_first(data, ' ');
    string8 b = string_substring(data, 0, space);
    out.b = atof(string_to_cstr(arena, b));
    return out;
}


Material* read_mtl_file(Arena* arena, string8 file_path) {
    LocalArena* local_arena = local_arena_alloc_create();

    string8 dirname = string_dirname(file_path);
    string8 content = read_file(local_arena->arena, file_path);

    Material* material = (Material*)arena_alloc_push(arena, sizeof(Material));

    u64 line_start = 0;
    for (u64 i=0; i<content.size; i++) {
        if ((content.buffer[i] == '\n') || (i == content.size-1)) {
            u64 line_end = i-1;
            if (i == content.size-1) {
                line_end = i;
            }
            string8 line = string_substring(content, line_start, line_end+1);
            if (string_starts_with(line, "newmtl ")) {
                string8 mtl_name = string_substring(line, 7, line.size);
                string8 name = string_copy_to_arena(arena, mtl_name);
                material->name = name;
            } else if (string_starts_with(line, "Ns ")) {
                string8 data = string_substring(line, 3, line.size);
                char* c_data = string_to_cstr(local_arena->arena, data);
                material->Ns = atof(c_data);
            } else if (string_starts_with(line, "Ka ")) {
                string8 data = string_substring(line, 3, line.size);
                material->Ka = string_to_f32x3(arena, data);
            } else if (string_starts_with(line, "Kd ")) {
                string8 data = string_substring(line, 3, line.size);
                material->Kd = string_to_f32x3(arena, data);
            } else if (string_starts_with(line, "Ks ")) {
                string8 data = string_substring(line, 3, line.size);
                material->Ks = string_to_f32x3(arena, data);
            } else if (string_starts_with(line, "Ke ")) {
                string8 data = string_substring(line, 3, line.size);
                material->Ke = string_to_f32x3(arena, data);
            } else if (string_starts_with(line, "Ni ")) {
                string8 data = string_substring(line, 3, line.size);
                char* c_data = string_to_cstr(local_arena->arena, data);
                material->Ni = atof(c_data);
            } else if (string_starts_with(line, "d ")) {
                string8 data = string_substring(line, 2, line.size);
                char* c_data = string_to_cstr(local_arena->arena, data);
                material->d = atof(c_data);
            } else if (string_starts_with(line, "illum ")) {
                string8 data = string_substring(line, 6, line.size);
                char* c_data = string_to_cstr(local_arena->arena, data);
                material->illum = atol(c_data);
            } else if (string_starts_with(line, "map_Kd ")) {
                string8 filename = string_substring(line, 7, line.size);
                string8 complete_path = string_join_strings(local_arena->arena, dirname, filename);
                material->map_Kd = read_image_file(arena, complete_path);
            } else if (string_starts_with(line, "map_Bump ")) {
                string8 filename = string_substring(line, 9, line.size);
                string8 complete_path = string_join_strings(local_arena->arena, dirname, filename);
                material->map_Bump = read_image_file(arena, complete_path);
            } else if (string_starts_with(line, "map_Ks ")) {
                string8 filename = string_substring(line, 7, line.size);
                string8 complete_path = string_join_strings(local_arena->arena, dirname, filename);
                material->map_Ks = read_image_file(arena, complete_path);
            }
            line_start = i+1;
        }
    }

    local_arena_alloc_reset(local_arena);
    return material;
}

/*
    From wiki
    symbol meanings:
    # -> comments
    v x y z [w=1] -> vertex coords
    vn x y z -> vertex normals (seems to have n instead)
    vt u [v=0] [w=0] -> texture coordinates 2D sometimes 3D
    vp u [v] [w] -> vertex parameter
    f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ... -> face definition usually a triangle     index 1-based not all are present (vt vn)
    l a b c d e -> line element

    mtllib [external .mtl file name]
    usemtl [material name]

    o => object name
     g => group name

    s 1  => smooth shading
     s off

    Using fgets()? and sscanf()?
*/
Model* read_obj_model_file(Arena* arena, string8 file_path) {
    LocalArena* local_arena = local_arena_alloc_create();

    string8 dirname = string_dirname(file_path);
    string8 file = read_file(local_arena->arena, file_path);
    u64 checkpoint = arena_alloc_checkpoint(local_arena->arena);

    u64 start_line = 0;
    u64 end_line   = 0;
    u64 size_line  = 0;

    Arena* vertices_arena   = arena_alloc_create(1*GiB);
    Arena* tex_coords_arena = arena_alloc_create(1*GiB);
    Arena* normals_arena    = arena_alloc_create(1*GiB);
    Arena* faces_arena      = arena_alloc_create(1*GiB);

    Vector* vertices   = vector_alloc_create(vertices_arena, sizeof(Vertex));
    Vector* tex_coords = vector_alloc_create(tex_coords_arena, sizeof(TexCoord));
    Vector* normals    = vector_alloc_create(normals_arena, sizeof(Normal));
    Vector* faces      = vector_alloc_create(faces_arena, sizeof(Face));

    u32 line_buffer_length = 128;
    char* line_buffer = (char*)arena_alloc_push(local_arena->arena, line_buffer_length);
    // string8 mat_file = {0};
    Material* material = 0;

    for (u64 i=0; i<file.size; i++) {
        if (file.buffer[i] == '\n' || (i==file.size-1)) {
            // Get current line
            end_line = i-1;
            if (i == file.size-1) {
                end_line = i;
            }
            size_line = end_line - start_line + 1;

            if (size_line >= 3) {
                // Make line_buffer bigger if needed
                if (size_line >= line_buffer_length) {         // = because we add a \0 at the end of the string
                    // arena_alloc_restore(local_arena->arena, checkpoint);         // Should be done but I want to use the local arena for other stuff and don't want to lose it all
                    line_buffer_length = 2*size_line;
                    line_buffer = (char*)arena_alloc_push(local_arena->arena, line_buffer_length);
                }
                memcpy((void*)line_buffer, (void*)&file.buffer[start_line], size_line);
                line_buffer[size_line] = '\0';

                // Process the line
                char first = line_buffer[0];
                char second = line_buffer[1];
                char third = line_buffer[2];
                if (first == 'v') {                                                 // Could be v vn vt vp
                    if (second == ' ') {
                        // process vertex
                        Vertex v = {};
                        sscanf(line_buffer, "v %f %f %f", &v.x, &v.y, &v.z);
                        vector_alloc_push(vertices, (void*)&v);
                    } else if (second == 'n') {
                        // process normal
                        Normal n = {};
                        sscanf(line_buffer, "vn %f %f %f", &n.x, &n.y, &n.z);
                        vector_alloc_push(normals, (void*)&n);

                    } else if (second == 't') {
                        // process texture
                        TexCoord t = {};
                        sscanf(line_buffer, "vt %f %f", &t.u, &t.v);
                        vector_alloc_push(tex_coords, (void*)&t);
                    } else if (second == 'p') {
                        // process vertex parameter
                    }
                } else if (first == 'f') {
                    Face f = {};
                    sscanf(line_buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                            &f.v[0], &f.vt[0], &f.vn[0],
                            &f.v[1], &f.vt[1], &f.vn[1],
                            &f.v[2], &f.vt[2], &f.vn[2]);
                    vector_alloc_push(faces, (void*)&f);
                } else if (first == 'o') {                                          // Object
                    // process object
                    // if (++tmp_o_count>1) break;
                } else if (first == 'g') {                                          // Group
                    // process group
                } else if (first == 's') {                                          // smooth shading
                    if (third == 'o') {
                        // process shading off
                    } else {
                        // process shading (intensity?)
                    }
                } else if (first == 'm') {                                          // mtllib (external .mtl file name)
                    string8 l = { .buffer=(u8*)&file.buffer[start_line], .size=size_line };
                    string8 mtllib = string_from_cstr(local_arena->arena, "mtllib");

                    if (string_starts_with(l, mtllib)) {
                        string8 mat_file = string_substring(l, string_find_first(l, ' ')+1, l.size);
                        string8 complete_mat_path = string_join_strings(local_arena->arena, dirname, mat_file);
                        material = read_mtl_file(arena, complete_mat_path);
                    }
                } else if (first == 'u') {                                          // usemtl (material name)
                } else if (first == '\n' || first == '#') {                         // new lines or comments
                }
            }

            start_line = i+1;
        }
    }
    arena_alloc_restore(local_arena->arena, checkpoint);

    Model* model = (Model*)arena_alloc_push(arena, sizeof(Model));
    model->material = material;
    model->vertices = vector_alloc_copy_to_arena(arena, vertices);
    model->faces = vector_alloc_copy_to_arena(arena, faces);
    model->normals = vector_alloc_copy_to_arena(arena, normals);
    model->tex_coords = vector_alloc_copy_to_arena(arena, tex_coords);

    arena_alloc_free(vertices_arena);
    arena_alloc_free(tex_coords_arena);
    arena_alloc_free(normals_arena);
    arena_alloc_free(faces_arena);

    local_arena_alloc_reset(local_arena);
    return model;
}

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

        // h - y so that top left is 0, 0
        u32 linear = (u32)(h-tmp.y)*w + (u32)tmp.x;
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


void draw_model_wireframe(Model* model, u32 w, u32 h, u32* framebuffer) {
    for (u64 i=0; i<vector_alloc_count(model->faces); ++i) {
        Face* f = (Face*)vector_alloc_get(model->faces, i);

        // Only use the x y components atm
        // Can work with perspective and camera later
        Vertex a = *(Vertex*)vector_alloc_get(model->vertices, f->v[0]-1);
        Vertex b = *(Vertex*)vector_alloc_get(model->vertices, f->v[1]-1);
        Vertex c = *(Vertex*)vector_alloc_get(model->vertices, f->v[2]-1);

        // Scale to center of the screen
        a.x = (a.x * 0.2f + 0.5f) * w;
        a.y = (a.y * 0.2f + 0.5f) * h;
        b.x = (b.x * 0.2f + 0.5f) * w;
        b.y = (b.y * 0.2f + 0.5f) * h;
        c.x = (c.x * 0.2f + 0.5f) * w;
        c.y = (c.y * 0.2f + 0.5f) * h;

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

f32 ceilf32(f32 d) {
    f32 trunc = (f32)((s32)d);
    if (trunc < d) {
        return trunc+1.0f;
    } else {
        return trunc;
    }
}

void fill_flat_top_triangle(Vertex* a, Vertex* b, Vertex* c, f32 zmid, u32 w, u32 h, u32* framebuffer, f32* zbuffer, u32 color) {
    f32 ay = a->y;
    f32 by = b->y;
    f32 ax = a->x;
    f32 bx = b->x;
    f32 cx = c->x;

    f32 y_delta_inv = 1.0f / (by - ay);
    f32 ad_inv_slope;
    f32 ae_inv_slope;

    if (bx <= cx) {
        ad_inv_slope = (bx - ax) * y_delta_inv;
        ae_inv_slope = (cx - ax) * y_delta_inv;
    } else {
        ad_inv_slope = (cx - ax) * y_delta_inv;
        ae_inv_slope = (bx - ax) * y_delta_inv;
    }

    u32 ys = ay<0 ? 0 : (u32)ceilf32(ay);
    u32 ye = by>h ? h : (u32)ceilf32(by);

    f32 xs = ax + (ys - ay) * ad_inv_slope;
    f32 xe = ax + (ys - ay) * ae_inv_slope;

    // printf("y: [%.3f,%.3f] -> [%d,%d]\n", ay, by, ys, ye);
    // printf("x: [%.3f,%.3f]           \n", xs, xe);

    for (u32 y=ys; y<ye; y++) {
        u32 x_start = xs<0 ? 0 : (u32)ceilf32(xs);
        u32 x_end   = xe>w ? w : (u32)ceilf32(xe);

        u32 offset = w*(h-1-y);
        for (u32 x=x_start; x<x_end; x++) {
            u32 off = offset + x;
            f32* zpix = zbuffer + off;

            if (*zpix < zmid) {
                *zpix = zmid;
                u32* pixel = framebuffer + off;
                *pixel = color;
            }
        }
        xs += ad_inv_slope;
        xe += ae_inv_slope;
    }
}

void fill_flat_bottom_triangle(Vertex* a, Vertex* b, Vertex* c, f32 zmid, u32 w, u32 h, u32* framebuffer, f32* zbuffer, u32 color) {
    f32 ay = a->y;
    f32 by = b->y;
    f32 cy = c->y;
    f32 ax = a->x;
    f32 bx = b->x;
    f32 cx = c->x;

    f32 dc_inv_slope;
    f32 ec_inv_slope;
    f32 dx;
    f32 ex;

    if (ax <= bx) {
        dc_inv_slope = (cx - ax) / (cy - ay);
        ec_inv_slope = (cx - bx) / (cy - by);
        dx = ax;
        ex = bx;
    } else {
        dc_inv_slope = (cx - bx) / (cy - by);
        ec_inv_slope = (cx - ax) / (cy - ay);
        dx = bx;
        ex = ax;
    }

    u32 ys = ay<0 ? 0 : (u32)ceilf32(ay);
    u32 ye = cy>h ? h : (u32)ceilf32(cy);

    f32 xs = dx + (ys - ay) * dc_inv_slope;
    f32 xe = ex + (ys - ay) * ec_inv_slope;

    for (u32 y=ys; y<ye; y++) {
        u32 x_start = xs<0 ? 0 : (u32)ceilf32(xs);
        u32 x_end   = xe>w ? w : (u32)ceilf32(xe);

        u32 offset = w*(h-1-y);
        for (u32 x=x_start; x<x_end; x++) {
            u32 off = offset + x;
            f32* zpix = zbuffer + off;
            if (*zpix < zmid) {
                *zpix = zmid;
                u32* pixel = framebuffer + off;
                *pixel = color;
            }
        }
        xs += dc_inv_slope;
        xe += ec_inv_slope;
    }
}

void fill_triangle_scanline(u32* framebuffer, f32* zbuffer, u32 w, u32 h, Vertex* v1, Vertex* v2, Vertex* v3, u32 color) {
    /*
     * f(t) = A + t * AB            t in [0, 1]
     *
     * x = A_x + t * AB_x
     * y = A_y + t * AB_y
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
     */

    Vertex* a = v1;
    Vertex* b = v2;
    Vertex* c = v3;

    // Sort in order of ascending y: a.y<b.y<c.y
    Vertex* t;
    if (b->y < a->y) { t = a; a = b; b = t; }
    if (c->y < a->y) { t = a; a = c; c = t; }
    if (c->y < b->y) { t = b; b = c; c = t; }

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

    f32 zmid = (a->z+b->z+c->z) / 3;

    if (b->y == c->y) {
        fill_flat_top_triangle(a, b, c, zmid, w, h, framebuffer, zbuffer, color);
    } else if (a->y == b->y) {
        fill_flat_bottom_triangle(a, b, c, zmid, w, h, framebuffer, zbuffer, color);
    } else {
        f32 ab_inv_slope = (b->x - a->x) / (b->y - a->y);
        f32 ac_inv_slope = (c->x - a->x) / (c->y - a->y);
        f32 bc_inv_slope = (c->x - b->x) / (c->y - b->y);
        f32 extra_x = a->x + (b->y - a->y) * ac_inv_slope;

        Vertex extra = {
            .x = extra_x,
            .y = b->y,
            .z = zmid,
        };

        if (b->y > 0) {
            fill_flat_top_triangle(a, b, &extra, zmid, w, h, framebuffer, zbuffer, color);
        }
        if (b->y < h) {
            fill_flat_bottom_triangle(&extra, b, c, zmid, w, h, framebuffer, zbuffer, color);
        }
    }
}


void draw_model(Model* model, u32 w, u32 h, u32* framebuffer, f32* zbuffer) {
    static bool swapper = false;

    f32 minx =  10000;
    f32 maxx = -10000;
    f32 miny =  10000;
    f32 maxy = -10000;
    f32 minz =  10000;
    f32 maxz = -10000;
    for (u64 i=0; i<vector_alloc_count(model->faces); ++i) {
        Face* f = (Face*)vector_alloc_get(model->faces, i);

        // Only use the x y components atm
        // Can work with perspective and camera later
        Vertex a = *(Vertex*)vector_alloc_get(model->vertices, f->v[0]-1);
        Vertex b = *(Vertex*)vector_alloc_get(model->vertices, f->v[1]-1);
        Vertex c = *(Vertex*)vector_alloc_get(model->vertices, f->v[2]-1);

        minx = minx < a.x ? minx : a.x;
        minx = minx < b.x ? minx : b.x;
        minx = minx < c.x ? minx : c.x;
        maxx = maxx > a.x ? maxx : a.x;
        maxx = maxx > b.x ? maxx : b.x;
        maxx = maxx > c.x ? maxx : c.x;

        miny = miny < a.y ? minx : a.y;
        miny = miny < b.y ? minx : b.y;
        miny = miny < c.y ? minx : c.y;
        maxy = maxy > a.y ? maxx : a.y;
        maxy = maxy > b.y ? maxx : b.y;
        maxy = maxy > c.y ? maxx : c.y;

        minz = minz < a.z ? minz : a.z;
        minz = minz < b.z ? minz : b.z;
        minz = minz < c.z ? minz : c.z;
        maxz = maxz > a.z ? maxz : a.z;
        maxz = maxz > b.z ? maxz : b.z;
        maxz = maxz > c.z ? maxz : c.z;
    }

    for (u64 i=0; i<vector_alloc_count(model->faces); ++i) {
        Face* f = (Face*)vector_alloc_get(model->faces, i);

        // Only use the x y components atm
        // Can work with perspective and camera later
        Vertex a = *(Vertex*)vector_alloc_get(model->vertices, f->v[0]-1);
        Vertex b = *(Vertex*)vector_alloc_get(model->vertices, f->v[1]-1);
        Vertex c = *(Vertex*)vector_alloc_get(model->vertices, f->v[2]-1);

        // Scale to center of the screen
        a.x = (a.x - minx) * h / (maxy-miny);
        b.x = (b.x - minx) * h / (maxy-miny);
        c.x = (c.x - minx) * h / (maxy-miny);

        a.y = (a.y - 0.5f*miny) * h / (maxy-miny);
        b.y = (b.y - 0.5f*miny) * h / (maxy-miny);
        c.y = (c.y - 0.5f*miny) * h / (maxy-miny);

        a.z = (a.z - minz) / (maxz-minz);
        b.z = (b.z - minz) / (maxz-minz);
        c.z = (c.z - minz) / (maxz-minz);

        // 0xAABBGGRR
        u32 col = (0xFF & (u32)(220*(a.z + b.z + c.z)/3));

        // fill_triangle_bbox_triangle_check(framebuffer, zbuffer, w, h, &a, &b, &c, col);
        // if (swapper)
        // fill_triangle_line_sweep_reference(framebuffer, zbuffer, w, h, &a, &b, &c, col);
        // else
        fill_triangle_scanline(framebuffer, zbuffer, w, h, &a, &b, &c, col);


    }
    swapper = !swapper;
}
