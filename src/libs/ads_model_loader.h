#ifndef ADS_MODEL_LOADER_H
#define ADS_MODEL_LOADER_H

#include "libs/ads_string.h"
#include "memory/allocators.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef f32x3 Vec3f;

typedef struct Texture Texture;
struct Texture {
    unsigned int* data;
    unsigned short width;
    unsigned short height;
    unsigned char components;
};

typedef struct {
    StringView name;        // newmtl

    Vec3f Ka;               // ambiant color
    Vec3f Kd;               // diffuse color
    Vec3f Ks;               // specular color
    Vec3f Ke;               // emissive color

    float Ns;               // specular exponent
    float Ni;               // index of refraction
    // float d;                // dissolve: Transparency = 1-d
    float Tr;               // Transparency
    float Tf;               // Transmission filter
    uint32_t illum;         // illumination

    // Textures
    StringView sv_map_Ka;      // ambient color
    StringView sv_map_Kd;      // diffuse texture
    StringView sv_map_Ks;      // specular texture
    StringView sv_map_Bump;    // normal map
    StringView sv_map_d;       // alpha texture map

    Texture map_Ka;
    Texture map_Kd;
    Texture map_Ks;
    Texture map_Bump;
    Texture map_d;
} ObjMaterial;

typedef struct {
    uint32_t v_indices[3];
    uint32_t vt_indices[3];
    uint32_t vn_indices[3];

    int material_index;
    int shading_group;
} ObjFace;

typedef struct {
    String name;
    int first_face_index;
    int face_count;

    // Easier to mark material for each group than for each face
    int material_index;
} ObjGroup;

typedef struct ObjModel ObjModel;
struct ObjModel {
    Vec3f* vertices;
    Vec3f* texcoords;
    Vec3f* normals;
    ObjFace* faces;
    ObjGroup* groups;
    ObjMaterial* materials;

    uint32_t n_vertices;
    uint32_t n_texcoords;
    uint32_t n_normals;
    uint32_t n_faces;
    uint32_t n_groups;
    uint32_t n_materials;

    // TODO(alex): There can be several mtllib => fix
    StringView mtllib_name;
};

typedef ObjModel Model;

// typedef struct {
// } Model;

Model*   model_read(Arena* arena, StringView filepath);
#ifdef __cplusplus
}
#endif

#endif // ADS_MODEL_LOADER_H
