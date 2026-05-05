#ifndef ADS_RENDERER_H
#define ADS_RENDERER_H

#include "base/base.h"
#include "libs/ads_model_loader.h"

// tmp?
void print(f32x3* v);

typedef void (*FragmentShader)(
    void* shader_ctx,
    f32x3* a,
    f32x3* b,
    f32x3* c,
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

void draw_model_wireframe(ObjModel* model, u32 w, u32 h, u32* framebuffer);
// draw_model()
void draw_scene(Scene* scene, u32* framebuffer, f32* zbuffer, u32 w, u32 h);

#endif // ADS_RENDERER_H
