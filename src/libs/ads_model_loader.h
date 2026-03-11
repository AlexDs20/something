#ifndef _ADS_MODEL_LOADER_H_
#define _ADS_MODEL_LOADER_H_

#include "memory/allocators.h"
#include "libs/ads_string.h"
#include "platform/io.h"

typedef struct {
    float x, y, z;
} Vec3f;

typedef struct {
    float u, v;
} Vec2f;

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
    StringView map_Ka;      // ambient color
    StringView map_Kd;      // diffuse texture
    StringView map_Ks;      // specular texture
    StringView map_Bump;    // normal map
    StringView map_d;       // alpha texture map
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

typedef struct {
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
} ObjModel;

// typedef struct {
// } Model;

ObjModel*   model_read(Arena* arena, StringView filepath);
// ObjModel*   model_parse_obj(Arena* arena, StringView filepath, StringView base_dir);
// ObjModel*   model_create_default_model(Arena* arena);
// ObjModel*   model_convert_from_obj(Arena* arena, ObjModel* obj_model);


#endif // _ADS_MODEL_LOADER_H_

