#include <cstdlib>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "renderer/renderer.h"

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

Arena* read_file(char* file_path, bool trim) {
    Arena* arena = arena_alloc_create(4*1024);
    if (!arena) {
        printf("Failed to create the initial arena\n");
        return(0);
    }

    FILE* file = fopen(file_path, "r");

    if (!file) {
        printf("Could not open file %s\n", file_path);
        arena_alloc_free(arena);
        return(0);
    }

    u64 chunk_size = 4*1024;
    u64 bytes_read;
    do {
        // Allocate more if needed
        if (arena_alloc_remaining(arena) < chunk_size) {
            Arena* arena2 = arena_alloc_create(2*arena->capacity);
            if (!arena2) {
                printf("Could not allocate enough memory!\n");
                arena_alloc_free(arena);
                return(0);
            }
            arena_alloc_copy(arena2, arena);
            arena_alloc_free(arena);
            arena = arena2;
        }

        bytes_read = fread(arena->buffer+arena->top, 1, chunk_size, file);
        if (bytes_read < chunk_size && ferror(file)) {
            printf("Failed while reading the file!\n");
            arena_alloc_free(arena);
            return(0);
        }
        arena->top += bytes_read;
    } while (bytes_read == chunk_size);

    int error = fclose(file);
    if (error) {
        printf("fclose failed: %s\n", strerror(errno));
        arena_alloc_free(arena);
        return(0);
    }

    // Make it just the right size
    if (trim && arena_alloc_remaining(arena)>0) {
        Arena* arena2 = arena_alloc_create(arena->top);
        if (!arena2) {
            printf("Failed to trim the arena\n");
            arena_alloc_free(arena);
            return(0);
        }
        arena_alloc_copy(arena2, arena);
        arena_alloc_free(arena);
        arena = arena2;
    }
    return arena;
}

Model* parse_obj_content(Arena* file) {
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
    u64 start_line = 0;
    u64 end_line = 0;
    u64 size_line = 0;

    // TODO(alex): Make it so that it rescales instead of overflowing.
    Vector* vertices     = vector_alloc_create(1000, sizeof(Vertex));
    Vector* tex_coords   = vector_alloc_create(1000, sizeof(TexCoord));
    Vector* normals      = vector_alloc_create(1000, sizeof(Normal));
    Vector* faces        = vector_alloc_create(1000, sizeof(Face));

    u32 line_buffer_length = 128;
    char* line_buffer = (char*)malloc(line_buffer_length);
    for (u64 i=0; i<file->top; i++) {
        if (file->buffer[i] == '\n' || (i==file->top-1)) {
            // Get current line
            end_line = i;
            size_line = end_line - start_line + 1;

            if (size_line >= 3) {
                // Make line_buffer bigger if needed
                if (size_line >= line_buffer_length) {         // = because we add a \0 at the end of the string
                    free(line_buffer);
                    line_buffer = (char*)malloc(2*size_line);
                    line_buffer_length = 2*size_line;
                }
                memcpy((void*)line_buffer, (void*)&file->buffer[start_line], size_line);
                line_buffer[size_line] = '\0';

                // Process the line
                char first = line_buffer[0];
                if (first == 'v') {                                                 // Could be v vn vt vp
                    char second = line_buffer[1];
                    if (second == ' ') {
                        // process vertex
                        Vertex v = {};
                        sscanf(line_buffer, "v %f %f %f\n", &v.x, &v.y, &v.z);
                        vector_alloc_push(vertices, (void*)&v);
                    } else if (second == 'n') {
                        // process normal
                        Normal n = {};
                        sscanf(line_buffer, "vn %f %f %f\n", &n.x, &n.y, &n.z);
                        vector_alloc_push(normals, (void*)&n);

                    } else if (second == 't') {
                        // process texture
                        TexCoord t = {};
                        sscanf(line_buffer, "vt %f %f\n", &t.u, &t.v);
                        vector_alloc_push(tex_coords, (void*)&t);
                    } else if (second == 'p') {
                        // process vertex parameter
                    }
                } else if (first == 'f') {
                    Face f = {};
                    sscanf(line_buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
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
                    char third = line_buffer[2];
                    if (third == 'o') {
                        // process shading off
                    } else {
                        // process shading (intensity?)
                    }
                } else if (first == 'm') {                                          // mtllib (external .mtl file name)
                } else if (first == 'u') {                                          // usemtl (material name)
                } else if (first == '\n' || first == '#') {                         // new lines or comments
                }
            }

            start_line = i+1;
        }
    }

    free(line_buffer);

    Model* model = (Model*)malloc(sizeof(Model));
    // vector_alloc_trim(vertices);
    // vector_alloc_trim(faces);
    // vector_alloc_trim(normals);
    // vector_alloc_trim(tex_coords);
    model->vertices = vertices;
    model->faces = faces;
    model->normals = normals;
    model->tex_coords = tex_coords;
    return model;
}

void free_model(Model* model) {
    vector_alloc_free(model->tex_coords);
    vector_alloc_free(model->vertices);
    vector_alloc_free(model->faces);
    vector_alloc_free(model->normals);
    free(model);
}

Model* read_model_file(char* file_path) {
    Arena* arena = read_file(file_path, true);
    Model* model = parse_obj_content(arena);
    arena_alloc_free(arena);
    return model;
}

static float
f32abs(float a) {
    union {
        float f;
        u32 i;
    } u;
    u.f = a;
    u.i &= 0x7fffffff;
    return u.f;
}

void draw_line(u32* framebuffer, u32 w, u32 h, Vertex* a, Vertex* b, Color c) {
    float dx = b->x-a->x;
    float dy = b->y-a->y;

    u32 steps = f32abs(dx)>f32abs(dy) ? f32abs(dx) : f32abs(dy);

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
    for (int i=0; i<vector_alloc_count(model->faces); ++i) {
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

void fill_triangle_line_sweep(u32* framebuffer, u32 w, u32 h, Vertex* a, Vertex* b, Vertex* c, u32 color) {
    if (a->y > b->y) { swap_vertices(a, b); }
    if (b->y > c->y) { swap_vertices(b, c); }
    if (a->y > b->y) { swap_vertices(a, b); }

    int ylow = (int)a->y;
    int ymid = (int)b->y;
    int yhigh = (int)c->y;

    for (int y=ylow; y<=yhigh; y++) {
        if (y < 0 || y >= h) continue;

        int xs;
        int xe;

        if (ylow == yhigh) {
            xs = (int)a->x;
        } else {
            xs = (int)(a->x + (c->x-a->x) * (y - ylow)/(yhigh-ylow));
        }

        if (y<=ymid) {
            if (ymid == ylow) {
                xe = (int)b->x;
            } else {
                xe = (int)(a->x + (b->x-a->x) * (y - ylow)/(ymid-ylow));
            }

        } else {
            if (yhigh == ymid) {
                xe = (int)c->x;
            } else {
                xe = (int)(b->x + (c->x-b->x) * (y - ymid)/(yhigh-ymid));
            }
        }
        if (xs>xe) { int tmp = xs; xs = xe; xe = tmp; }
        for (int x=xs; x<=xe; x++) {
            if (x < 0 || x >= w) continue;
            u32* pixel = framebuffer + w*(h-y) + x;
            *pixel = color;
        }
    }
}


void fill_triangle_line_sweep_reference(u32* framebuffer, u32 w, u32 h, Vertex* a, Vertex* b, Vertex* c, u32 color) {
    if (a->y==b->y && a->y==c->y) return; // i dont care about degenerate triangles
    if (a->y>b->y) swap_vertices(a, b);
    if (a->y>c->y) swap_vertices(a, c);
    if (b->y>c->y) swap_vertices(b, c);

    s32 ax = (u32)a->x;
    s32 ay = (u32)a->y;
    s32 bx = (u32)b->x;
    s32 by = (u32)b->y;
    s32 cx = (u32)c->x;
    s32 cy = (u32)c->y;

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
            if (ay+i<0 || ay+i>=h || j<0 || j>=w) continue;
            u32 linear = w*(h- (ay+i)) + j;
            u32* pixel = framebuffer + linear;
            *pixel = color;
            // image.set(j, ay+i, color); // attention, due to int casts a->y+i != A.y
        }
    }
}

u32 random_color(u64 v) {
    u32 c = 0xFFFFFF * (v / (f64)RAND_MAX);
    return c;
}

void draw_model(Model* model, u32 w, u32 h, u32* framebuffer) {
    for (int i=0; i<vector_alloc_count(model->faces); ++i) {
        Face* f = (Face*)vector_alloc_get(model->faces, i);

        // if (i%50==0)
        {
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

        // u32 col = 0xFFA500;
        u32 col = random_color(
                (u64)vector_alloc_get(model->vertices, f->v[0]-1)
                + (u64)vector_alloc_get(model->vertices, f->v[1]-1)
                - (u64)vector_alloc_get(model->vertices, f->v[2]-1));

        fill_triangle_line_sweep(framebuffer, w, h, &a, &b, &c, col);
        // fill_triangle_line_sweep_reference(framebuffer, w, h, &a, &b, &c, col);
        }
    }
}
