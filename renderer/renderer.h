#ifndef _RENDERER_H
#define _RENDERER_H

#include "utils/defines.h"
#include "utils/allocators.h"

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

// tmp?
void print(Vertex* v);
void print(Normal* n);
void print(TexCoord* t);
void print(Face* t);

typedef struct {
    Vector* vertices;
    Vector* tex_coords;
    Vector* normals;
    Vector* faces;
    Vector* mat;
} Model;

Model* read_model_file(char* filepath);
void free_model(Model* model);

void draw_model_wireframe(Model* model, uint32 w, uint32 h, uint32* framebuffer);

#endif
