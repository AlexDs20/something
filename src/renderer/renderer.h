#ifndef ADS_RENDERER_H
#define ADS_RENDERER_H

#include "base/base.h"
#include "libs/ads_model_loader.h"

typedef struct {
    float u, v, w;
    float nx, ny, nz;
    // float r, g, b;
} VertexAttrs;

// tmp?
void print(f32x3* v);
void print(VertexAttrs* va);

typedef void (*FragmentShader)(
    void* shader_ctx,
    f32x3* a,
    f32x3* b,
    f32x3* c,
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
    f32x4x4* world;
} TextureContext;

void shader_frag_color  (void* shader_ctx,
        f32x3* a, f32x3* b, f32x3* c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        f32 w0, f32 w1, f32 w2,
        u32 x, u32 y,
        u32 w, u32 h, f32* zbuffer, u32* framebuffer);
void shader_frag_depth  (void* shader_ctx, f32x3* a, f32x3* b, f32x3* c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        f32 w0, f32 w1, f32 w2, u32 x, u32 y, u32 w, u32 h, f32* zbuffer, u32* framebuffer);
void shader_frag_texture(void* shader_ctx, f32x3* a, f32x3* b, f32x3* c,
        VertexAttrs* va, VertexAttrs* vb, VertexAttrs* vc,
        f32 w0, f32 w1, f32 w2, u32 x, u32 y, u32 w, u32 h, f32* zbuffer, u32* framebuffer);


void draw_model_wireframe(ObjModel* model, u32 w, u32 h, u32* framebuffer);

void draw_model(Scene* scene, u32 w, u32 h, u32* framebuffer, f32* zbuffer, void* shader_context, FragmentShader frag_shader);

#endif // ADS_RENDERER_H
