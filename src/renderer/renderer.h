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

typedef struct {
    float u, v, w;
} TexCoord;

typedef struct {
    float x, y, z;
} Normal;

typedef struct {
    int v[3];
} Face;

typedef struct {
    string8 name;       // newmtl Scene_-_Root
    f32 Ns;             // Specular exponent Ns 225.000000
    f32x3 Ka;           // ambient color Ka 1.000000 1.000000 1.000000
    f32x3 Kd;           // diffuse color Kd 0.800000 0.800000 0.800000
    f32x3 Ks;           // specular color Ks 0.500000 0.500000 0.500000
    f32x3 Ke;           // emissive color Ke 0.0 0.0 0.0
    f32 Ni;             // index of refraction Ni 1.450000
    f32 d;              // dissolve / alpha d 1.000000
    u32 illum;          // illumination model illum 2
                        // 0. Color on and Ambient off
                        // 1. Color on and Ambient on
                        // 2. Highlight on
                        // 3. Reflection on and Ray trace on
                        // 4. Transparency: Glass on, Reflection: Ray trace on
                        // 5. Reflection: Fresnel on and Ray trace on
                        // 6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
                        // 7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
                        // 8. Reflection on and Ray trace off
                        // 9. Transparency: Glass on, Reflection: Ray trace off
                        // 10. Casts shadows onto invisible surfaces

    Image map_Kd;        // texture map diffuse map_Kd diffuse.jpg
    Image map_Bump;      // texture map bump map_Bump normal.png
    Image map_Ks;        // texture map specular map_Ks specular.jpg
} Material;

typedef struct {
    Material* material;             // TODO(alex): To change to a vector of materials
    Vector* vertices;
    Vector* vertex_attrs;
    // Vector* tex_coords;
    // Vector* normals;
    Vector* faces;
} Model;

// tmp?
void print(Vertex* v);
void print(VertexAttrs* va);
void print(Normal* n);
void print(TexCoord* t);
void print(Face* t);


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

Model* read_obj_model_file(Arena* arena, string8 filepath);

#include "libs/ads_model_loader.h"
void draw_model_wireframe(ObjModel* model, u32 w, u32 h, u32* framebuffer);

void draw_model(ObjModel* model, u32 w, u32 h, u32* framebuffer, f32* zbuffer, void* shader_context, FragmentShader frag_shader);

#endif
