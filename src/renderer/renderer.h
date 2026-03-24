#ifndef ADS_RENDERER_H
#define ADS_RENDERER_H

#include "base/base.h"

//#include "libs/ads_model_loader.h"
struct ObjModel;
struct Texture;
// #include "memory/allocators.h"
struct Arena;


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
    Texture* texture;
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


void draw_model_wireframe(ObjModel* model, u32 w, u32 h, u32* framebuffer);

void draw_model(ObjModel* model, u32 w, u32 h, u32* framebuffer, f32* zbuffer, void* shader_context, FragmentShader frag_shader);

#endif // ADS_RENDERER_H
