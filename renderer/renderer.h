#ifndef _RENDERER_H
#define _RENDERER_H

#include "utils/types.h"
#include "memory/allocators.h"

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    float u, v, w;
} TexCoord;

typedef struct {
    float x, y, z;
} Normal;

typedef struct {
    int v[3];
    int vt[3];
    int vn[3];
} Face;

typedef struct {

} Material;

typedef struct {
    Vector* vertices;
    Vector* tex_coords;
    Vector* normals;
    Vector* faces;
    Vector* mat;
} Model;

// tmp?
void print(Vertex* v);
void print(Normal* n);
void print(TexCoord* t);
void print(Face* t);

Model* read_model_file(Arena* arena, char* filepath);
void free_model(Model* model);

void draw_model_wireframe(Model* model, u32 w, u32 h, u32* framebuffer);

void draw_model(Model* model, u32 w, u32 h, u32* framebuffer, f32* zbuffer);

#endif
