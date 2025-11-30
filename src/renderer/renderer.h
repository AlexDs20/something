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

Model* read_obj_model_file(Arena* arena, string8 filepath);

void draw_model_wireframe(Model* model, u32 w, u32 h, u32* framebuffer);

void draw_model(Model* model, u32 w, u32 h, u32* framebuffer, f32* zbuffer);

#endif
