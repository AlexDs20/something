#ifndef ADS_MODEL_LOADER_H
#define ADS_MODEL_LOADER_H

#include "base/types.h"
#include "libs/ads_string.h"
#include "memory/allocators.h"

/*
 * The idea here is that the scene contains arrays of the different things it owns: materials, meshes, textures, objects
 *
 * Each object (instance) points to a mesh which has vertices and.
 * Each mesh (contains vertices and indices) is made of submeshes with same material and index range
 */

typedef struct Scene Scene;
typedef struct Object Object;
typedef struct AABB AABB;
typedef struct Mesh Mesh;
typedef struct SubMesh SubMesh;
typedef struct Vertex Vertex;
typedef struct Material Material;
typedef struct Texture Texture;
typedef struct ObjMaterial ObjMaterial;
typedef struct ObjFace ObjFace;
typedef struct ObjGroup ObjGroup;
typedef struct ObjModel ObjModel;


#ifdef __cplusplus
extern "C" {
#endif

struct Texture {
    unsigned int* data;
    unsigned short width;
    unsigned short height;
    unsigned char components;
};

struct ObjMaterial {
    StringView name;        // newmtl

    f32x3 Ka;               // ambiant color
    f32x3 Kd;               // diffuse color
    f32x3 Ks;               // specular color
    f32x3 Ke;               // emissive color
    f32x3 Tf;               // Transmission filter

    float Ns;               // specular exponent
    float Ni;               // index of refraction
    // float d;                // dissolve: Transparency = 1-d
    float Tr;               // Transparency
    uint32_t illum;         // illumination

    // Textures
    StringView sv_map_Ka;      // ambient color
    StringView sv_map_Kd;      // diffuse texture
    StringView sv_map_Ks;      // specular texture
    StringView sv_map_Bump;    // normal map
    StringView sv_map_d;       // alpha texture map

    struct Texture map_Ka;
    struct Texture map_Kd;
    struct Texture map_Ks;
    struct Texture map_Bump;
    struct Texture map_d;
};

struct ObjFace {
    uint32_t v_indices[3];
    uint32_t vt_indices[3];
    uint32_t vn_indices[3];

    int material_index;
    int smooth_shading;
};

struct ObjGroup {
    String name;
    int first_face_index;
    int face_count;

    // Easier to mark material for each group than for each face
    int material_index;
};

struct ObjModel {
    f32x3* vertices;
    f32x3* texcoords;
    f32x3* normals;
    struct ObjFace* faces;
    struct ObjGroup* groups;
    struct ObjMaterial* materials;

    uint32_t n_vertices;
    uint32_t n_texcoords;
    uint32_t n_normals;
    uint32_t n_faces;
    uint32_t n_groups;
    uint32_t n_materials;

    // TODO(alex): There can be several mtllib => fix
    StringView mtllib_name;
};

enum ShaderID {
    SHADER_ID_NIL,
    SHADER_ID_TEXTURE,
    SHADER_ID_COUNT,
};

struct Material {
    StringView name;        // newmtl

    f32x3 Ka;               // ambiant color
    f32x3 Kd;               // diffuse color
    f32x3 Ks;               // specular color
    f32x3 Ke;               // emissive color
    f32x3 Tf;               // Transmission filter

    float Ns;               // specular exponent
    float Ni;               // index of refraction
    // float d;                // dissolve: Transparency = 1-d
    float Tr;               // Transparency
    uint32_t illum;         // illumination

    // Textures
    struct Texture* map_Ka;
    struct Texture* map_Kd;
    struct Texture* map_Ks;
    struct Texture* map_Bump;
    struct Texture* map_d;

    enum ShaderID shader;
};

struct SubMesh {
    uint32_t start_index;
    uint32_t count;

    struct Material* mat;
};

struct Vertex {
    f32x3 position;
    f32x3 texcoords;        // f32x2 ?
    f32x3 normals;
};

struct Mesh {
    struct Vertex* vertices;
    uint32_t n_vertices;

    uint32_t* indices;
    uint32_t n_indices;

    struct SubMesh* submeshes;
    uint32_t n_submeshes;
};

struct AABB {
    f32x3 bb_min;
    f32x3 bb_max;
};

struct Object {
    struct Mesh* mesh;
    f32x4x4 transform;
    struct AABB bbox;
};

struct Scene {
    struct Mesh* meshes;
    uint32_t n_meshes;

    struct Material* mats;
    uint32_t n_mats;

    struct Texture* textures;
    uint32_t n_textures;

    struct Object* objects;
    uint32_t n_objects;
};

#ifdef ADS_DEBUG
#include "libs/ads_math.h"

static inline void
objface_debug_print(struct ObjFace* f) {
    printf("\nv  indices = (%u,%u,%u)", f->v_indices[0], f->v_indices[1], f->v_indices[2]);
    printf("\nvt indices = (%u,%u,%u)", f->vt_indices[0], f->vt_indices[1], f->vt_indices[2]);
    printf("\nvn indices = (%u,%u,%u)", f->vn_indices[0], f->vn_indices[1], f->vn_indices[2]);
    printf("\nmaterial index: %d", f->material_index);
    printf("\nsmooth shading: %d", f->smooth_shading);
}

static inline void
texture_debug_print(struct Texture* texture) {
    if (texture == NULL) {
        printf("NULL");
        return;
    }

    printf("Texture: %p, (%d,%d,%d)", (void*)texture->data, texture->width, texture->height, texture->components);

}

static inline void
material_debug_print(struct Material* mat) {
    printf("\nMaterial: %s", mat->name.buffer);
    printf("\nKa: "); f32x3_debug_print(mat->Ka);
    printf("\nKd: "); f32x3_debug_print(mat->Kd);
    printf("\nKs: "); f32x3_debug_print(mat->Ks);
    printf("\nKe: "); f32x3_debug_print(mat->Ke);
    printf("\nTf: "); f32x3_debug_print(mat->Tf);

    printf("\nNs: "); f32_debug_print(mat->Ns);
    printf("\nNi: "); f32_debug_print(mat->Ni);
    printf("\nTr: "); f32_debug_print(mat->Tr);
    printf("\nIllum: %d", mat->illum);

    printf("\nTexture map_Ka: "); texture_debug_print(mat->map_Ka);
    printf("\nTexture map_Kd: "); texture_debug_print(mat->map_Kd);
    printf("\nTexture map_Ks: "); texture_debug_print(mat->map_Ks);
    printf("\nTexture map_Bump: "); texture_debug_print(mat->map_Bump);
    printf("\nTexture map_d: "); texture_debug_print(mat->map_d);

    // printf(enum shader)
}

static inline void
submesh_debug_print(struct SubMesh* sm) {
    printf("Submesh indices: %d -> %d\n", sm->start_index, sm->start_index + sm->count);
    printf("Submesh material: \n");
    material_debug_print(sm->mat);
}

static inline void
mesh_debug_print(struct Mesh* mesh) {
    printf("Mesh:\n");
    printf("  Vertices: %d -> %p\n", mesh->n_vertices, (void*)mesh->vertices);
    printf("  Indices: %d -> %p\n", mesh->n_indices, (void*)mesh->indices);
    printf("  Submeshes: %d -> %p\n", mesh->n_submeshes, (void*)mesh->submeshes);
    for (uint32_t i=0; i<mesh->n_submeshes; ++i) {
        printf("  Submesh %d:", i);
        submesh_debug_print(&mesh->submeshes[i]);
        printf("\n");
    }
}

static inline void
object_debug_print(struct Object* obj) {
    mesh_debug_print(obj->mesh);
    f32x4x4_debug_print(obj->transform);
    // bbox_debug_print();
}

#else
#define objface_debug_print(f)
#endif

struct Scene* model_read(Arena* arena, StringView filepath);
#ifdef __cplusplus
}
#endif

#endif // ADS_MODEL_LOADER_H
