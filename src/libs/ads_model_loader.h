#ifndef _ADS_MODEL_LOADER_H_
#define _ADS_MODEL_LOADER_H_

#include "memory/allocators.h"
#include "libs/ads_string.h"

typedef struct {
    float x, y, z;
} Vec3f;

typedef struct {
    float u, v;
} Vec2f;

typedef struct {
    float x, y, z;
} ObjVertex;

typedef struct {
    float u, v, w;
    float nx, ny, nz;
} ObjVertexAttrs;

typedef struct {
    int vertices[3];
    // int vertex_count;        Assume it's always triangle
    int material_index;
    int shading_group;
} ObjFace;

typedef struct {
    String name;
    int first_face_index;
    int face_count;
} ObjGroup;

typedef struct {
} ObjModel;


typedef struct {
} Model;

Model*      model_read(Arena* arena, StringView filepath);
ObjModel*   model_parse_obj(Arena* arena, StringView filepath, StringView base_dir);
Model*      model_create_default_model(Arena* arena);
Model*      model_convert_from_obj(Arena* arena, ObjModel* obj_model);


#endif // _ADS_MODEL_LOADER_H_

