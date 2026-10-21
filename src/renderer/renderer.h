#ifndef ADS_RENDERER_H
#define ADS_RENDERER_H

#include "base/base.h"
#include "libs/ads_model_loader.h"

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


// TODO:
// void draw_model_wireframe(struct Object* model, u32 w, u32 h, u32* framebuffer);
// void draw_scene_wireframe(struct Scene* scene, f32x4x4 world, u32* framebuffer, f32* zbuffer, u32 w, u32 h);


// void draw_model(struct Object* obj, struct Material* mat, f32x4x4 world, u32* framebuffer, f32* zbuffer, u32 w, u32 h);
void draw_scene(struct Scene* scene, f32x4x4 world, u32* framebuffer, f32* zbuffer, u32 w, u32 h);

#endif // ADS_RENDERER_H
