#ifndef _RENDERER_H
#define _RENDERER_H

#include "utils/types.h"
#include "libs/ads_images.h"
#include "memory/allocators.h"

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    float u, v, w;
    float nx, ny, nz;
    // float r, g, b;
} VertexAttrs;

// tmp?
void print(Vertex* v);
void print(VertexAttrs* va);

typedef void (*FragmentShader)(
    void* shader_ctx,
    Vertex* a,
    Vertex* b,
    Vertex* c,
    VertexAttrs* va,
    VertexAttrs* vb,
    VertexAttrs* vc,
    f32 w0,
    f32 w1,
    f32 w2,
    u32 x,
    u32 y,
    u32 w,
    u32 h,
    f32* zbuffer,
    u32* framebuffer
);

typedef struct {
    u32 color;
} ColorContext;

typedef struct {
    Image* texture;
    u64 face_idx;
} TextureContext;

void shader_frag_color  (void* shader_ctx,
        Vertex* a, Vertex* b, Vertex* c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        f32 w0, f32 w1, f32 w2,
        u32 x, u32 y,
        u32 w, u32 h, f32* zbuffer, u32* framebuffer);
void shader_frag_depth  (void* shader_ctx, Vertex* a, Vertex* b, Vertex* c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        f32 w0, f32 w1, f32 w2, u32 x, u32 y, u32 w, u32 h, f32* zbuffer, u32* framebuffer);
void shader_frag_texture(void* shader_ctx, Vertex* a, Vertex* b, Vertex* c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        f32 w0, f32 w1, f32 w2, u32 x, u32 y, u32 w, u32 h, f32* zbuffer, u32* framebuffer);


#include "libs/ads_model_loader.h"
void draw_model_wireframe(ObjModel* model, u32 w, u32 h, u32* framebuffer);

void draw_model(ObjModel* model, u32 w, u32 h, u32* framebuffer, f32* zbuffer, void* shader_context, FragmentShader frag_shader);

#endif
