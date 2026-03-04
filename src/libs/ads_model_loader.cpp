#include "libs/ads_model_loader.h"
#include <stdio.h>

// TODO remove from here
#define GiB (1024*1024*1024)     // GiB

static ObjMaterial* read_mtl_file(Arena* arena, StringView filepath, uint32_t* material_count) {
    /*
     * Sources:
     * https://en.wikipedia.org/wiki/Wavefront_.obj_file
     * https://www.fileformat.info/format/wavefrontobj/egff.htm
     * https://paulbourke.net/dataformats/mtl/
     */
    LocalArena* local_arena = local_arena_alloc_create();
    String content = read_complete_file(local_arena->arena, filepath);
    StringView file = sv_from_string(content);

    // TODO: There are String in this struct which should first be allocated in another arena and the copied to this one in the end
    ObjMaterial dummy_mats = {0};

    ObjMaterial* first_mats = (ObjMaterial*)arena_alloc_push_struct(arena, &dummy_mats, sizeof(ObjMaterial));
    ObjMaterial* current_mats = first_mats;

    StringView space = sv_from_cstr(" ");
    StringView new_line = sv_from_cstr("\n");
    StringView newmtl = sv_from_cstr("newmtl");
    StringView line = sv_chop_by_delim_sv(&file, new_line);
    while (file.size != 0) {
        if (sv_starts_with(line, newmtl)) {
            line = sv_truncate_front(line, newmtl.size+1);
            // sv_print(line);
            // TODO: Implement push struct zero
            current_mats = (ObjMaterial*)arena_alloc_push_struct(arena, &dummy_mats, sizeof(ObjMaterial));
            *material_count += 1;
        }
        else if (sv_starts_with_cstr(line, "Ka ")) {        // ambient color
            line = sv_truncate_front(line, 3);
            sv_parse_f32(&line, &current_mats->Ka.x);
            sv_parse_f32(&line, &current_mats->Ka.y);
            sv_parse_f32(&line, &current_mats->Ka.z);
        }
        else if (sv_starts_with_cstr(line, "Kd ")) {        // diffuse color
            line = sv_truncate_front(line, 3);
            sv_parse_f32(&line, &current_mats->Kd.x);
            sv_parse_f32(&line, &current_mats->Kd.y);
            sv_parse_f32(&line, &current_mats->Kd.z);
        }
        else if (sv_starts_with_cstr(line, "Ks ")) {        // specular color
            line = sv_truncate_front(line, 3);
            sv_parse_f32(&line, &current_mats->Ks.x);
            sv_parse_f32(&line, &current_mats->Ks.y);
            sv_parse_f32(&line, &current_mats->Ks.z);
        }
        else if (sv_starts_with_cstr(line, "Ke ")) {        // emissive color
            line = sv_truncate_front(line, 3);
            sv_parse_f32(&line, &current_mats->Ke.x);
            sv_parse_f32(&line, &current_mats->Ke.y);
            sv_parse_f32(&line, &current_mats->Ke.z);
        }
        else if (sv_starts_with_cstr(line, "Ns ")) {        // specular exponent
            line = sv_truncate_front(line, 3);
            sv_parse_f32(&line, &current_mats->Ns);
        }
        else if (sv_starts_with_cstr(line, "Ni ")) {        // Index of refraction
            line = sv_truncate_front(line, 3);
            sv_parse_f32(&line, &current_mats->Ni);
        }
        // else if (sv_starts_with_cstr(line, "Tr ")) {        // Transparency
        // }
        else if (sv_starts_with_cstr(line, "d ")) {         // dissolve: Tr = 1-d
            line = sv_truncate_front(line, 3);
            sv_parse_f32(&line, &current_mats->d);
        }
        else if (sv_starts_with_cstr(line, "illum ")) {     // Illumination
            line = sv_truncate_front(line, 3);
            sv_parse_u32(&line, &current_mats->illum);
        }
        // else if (sv_starts_with_cstr(line, "Tf ")) {        // Transmission filter color
        // }
        // else if (sv_starts_with_cstr(line, "sharpness ")) {
        // }
        // else if (sv_starts_with_cstr(line, "map_Ka ")) {    // texture map ambient color
        // }
        else if (sv_starts_with_cstr(line, "map_Kd ")) {    // texture map diffuse color
            line = sv_truncate_front(line, 7);
            // TODO: handle string
        }
        else if (sv_starts_with_cstr(line, "map_Ks ")) {    // texture map specular color
            line = sv_truncate_front(line, 7);
            // TODO: handle string
        }
        // else if (sv_starts_with_cstr(line, "map_Ns ")) {    // specular highlight component
        // }
        // else if (sv_starts_with_cstr(line, "map_d ")) {     // alpha texture map
        // }
        else if (sv_starts_with_cstr(line, "map_bump ")) {  // map_bump and bump: same
            line = sv_truncate_front(line, 9);
            // TODO: handle string
        }
        else if (sv_starts_with_cstr(line, "map_Bump ")) {
            line = sv_truncate_front(line, 9);
            // TODO: handle string
        }
        else if (sv_starts_with_cstr(line, "bump ")) {
            line = sv_truncate_front(line, 9);
            // TODO: handle string
        }
        // else if (sv_starts_with_cstr(line, "disp ")) {      // displacement map
        // }
        // else if (sv_starts_with_cstr(line, "decal ")) {     // stencial decal texture
        // }
        // else if (sv_starts_with_cstr(line, "refl ")) {      // reflection map
        // }
        else if (sv_starts_with_cstr(line, "# ")) {
        }
        else if (sv_trim_front(line).size == 0) {           // If there are only spaces
        }
        // Extensions for physically based rendering
        // else if (sv_starts_with_cstr(line, "Pr ")) {        // roughness
        // }
        // else if (sv_starts_with_cstr(line, "map_Pr ")) {    // texture roughness
        // }
        // else if (sv_starts_with_cstr(line, "Pm ")) {        // metallic
        // }
        // else if (sv_starts_with_cstr(line, "map_Pm ")) {    // texture metallic
        // }
        // else if (sv_starts_with_cstr(line, "Ps ")) {        // sheen
        // }
        // else if (sv_starts_with_cstr(line, "map_Ps ")) {    // texture sheen
        // }
        // else if (sv_starts_with_cstr(line, "Pc ")) {        // clearcoat thickness
        // }
        // else if (sv_starts_with_cstr(line, "Pcr ")) {       // clearcoat roughness
        // }
        // else if (sv_starts_with_cstr(line, "Ke ")) {        // emissive
        // }
        // else if (sv_starts_with_cstr(line, "aniso ")) {     // anisotropy
        // }
        // else if (sv_starts_with_cstr(line, "anisor ")) {    // anisotropy rotation
        // }
        // else if (sv_starts_with_cstr(line, "norm ")) {      // normal map (RGB components represent XYZ components of the surface normal)
        // }
        else {
            String err = string_init_cstr(local_arena->arena, "Parsing of mtl file ");
            string_append_sv(local_arena->arena, &err, filepath);
            string_append_cstr(local_arena->arena, &err, " failed with line: ");
            string_append_sv(local_arena->arena, &err, line);
            printf("\n=====\n");
            string_print(&err);
            printf("\n=====\n");
            *(char*)0 = 0;
        }

        line = sv_chop_by_delim_sv(&file, new_line);
    }

    local_arena_alloc_reset(local_arena);
    return first_mats;
}

ObjModel* model_parse_obj(Arena* persist_arena, StringView file, StringView base_dir) {
    LocalArena* local_arena = local_arena_alloc_create();

    Arena* vertex_arena = arena_alloc_create(1*GiB);
    Vector* vec_vertex = vector_alloc_create(vertex_arena, sizeof(Vec3f));

    Arena* texcoords_arena = arena_alloc_create(1*GiB);
    Vector* vec_texcoords = vector_alloc_create(texcoords_arena, sizeof(Vec3f));

    Arena* normals_arena = arena_alloc_create(1*GiB);
    Vector* vec_normals = vector_alloc_create(normals_arena, sizeof(Vec3f));

    Arena* faces_arena = arena_alloc_create(1*GiB);
    Vector* vec_faces = vector_alloc_create(faces_arena, sizeof(ObjFace));

    Arena* materials_arena = arena_alloc_create(1*GiB);

    // TODO
    // arena_alloc_free(vertex_arena);
    // arena_alloc_free(texcoords_arena);
    // arena_alloc_free(normals_arena);
    // arena_alloc_free(faces_arena);
    // arena_alloc_free(materials_arena);

    Vec3f dummy_Vec3f = {0};
    ObjFace dummy_face = {0};
    // TODO: -1 as defaults? Or 0 and get default material in the array?
    int current_material_index = -1;
    StringView current_material_name;
    int current_shading_group = -1;

    // Setup the different types
    StringView new_line = sv_from_cstr("\n");
    StringView usemtl = sv_from_cstr("usemtl");
    StringView mtllib = sv_from_cstr("mtllib");

    StringView line = sv_chop_by_delim_sv(&file, new_line);
    while (file.size != 0) {
        if (sv_starts_with_char(line, 'v')) {
            Vec3f* v = NULL;
            int r;

            sv_chop_by(&line, 1);
            if (sv_starts_with_char(line, ' ')) {       // vertex
                line = sv_truncate_front(line, 2);
                v = (Vec3f*)vector_alloc_push(vec_vertex, &dummy_Vec3f);
            }
            else if (sv_starts_with_char(line, 't')) {  // texture coord
                line = sv_truncate_front(line, 3);
                v = (Vec3f*)vector_alloc_push(vec_texcoords, &dummy_Vec3f);
            }
            else if (sv_starts_with_char(line, 'n')) {  // vertex normal
                line = sv_truncate_front(line, 3);
                v = (Vec3f*)vector_alloc_push(vec_normals, &dummy_Vec3f);
            }
            // else if (sv_starts_with_char(line, 'p')) {  // parameter space vertices
            // }
            else {
                *((char*)0) = 0;
            }

            r = sv_parse_f32(&line, &v->x);
            r = sv_parse_f32(&line, &v->y);
            r = sv_parse_f32(&line, &v->z);
        }
        else if (sv_starts_with_char(line, 'f')) {      // face
            line = sv_truncate_front(line, 2);
            ObjFace* v = (ObjFace*)vector_alloc_push(vec_faces, &dummy_face);
            int r;

            StringView delim = sv_from_cstr("/");
            StringView sep;

            r = sv_parse_u32(&line, &v->v_indices[0]);
            sep = sv_chop_by(&line, 1);                 // sep could be '/' or ' '. if '/' => read vt and vn
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &v->v_indices[1]);

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &v->v_indices[2]);
                }
            }

            r = sv_parse_u32(&line, &v->vt_indices[0]);
            sep = sv_chop_by(&line, 1);
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &v->vt_indices[1]);

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &v->vt_indices[2]);
                }
            }

            r = sv_parse_u32(&line, &v->vn_indices[0]);
            sep = sv_chop_by(&line, 1);
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &v->vn_indices[1]);

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &v->vn_indices[2]);
                }
            }
        }
        else if (sv_starts_with_char(line, 's')) {      // smooth shading s 1  or s off
            line = sv_truncate_front(line, 2);
            if (sv_starts_with_cstr(line, "0") || sv_starts_with_cstr(line, "off")) {
                current_shading_group = 0;
            }
            else {
                int r = sv_parse_s32(&line, &current_shading_group);
            }
        }
        // else if (sv_starts_with_char(line, 'l')) {      // line element
        // }
        // else if (sv_starts_with_char(line, 'p')) {      // point element
        // }
        else if (sv_starts_with(line, usemtl)) {
            line = sv_truncate_front(line, usemtl.size+1);
            current_material_name = sv_trim_back(line);
            // TODO: Somehow get material id
        }
        else if (sv_starts_with(line, mtllib)) {
            line = sv_trim_back(sv_truncate_front(line, mtllib.size+1));

            String fp = string_init_sv(local_arena->arena, base_dir);
            string_append_sv(local_arena->arena, &fp, line);

            uint32_t material_count;
            StringView mtl_file = sv_from_string(fp);
            ObjMaterial* mats = read_mtl_file(materials_arena, mtl_file, &material_count);
        }
        else if (sv_starts_with_char(line, 'o')) {      // Object name
            StringView object_name = sv_truncate_front(line, 2);
        }
        else if (sv_starts_with_char(line, 'g')) {      // Group name (there can be many, in that case => data after belong to each group)
            StringView group_name = sv_truncate_front(line, 2);
        }
        else if (sv_starts_with_char(line, '#')) {
        }
        else {
            String err = string_init_cstr(local_arena->arena, "Parsing of obj file in dir: ");
            string_append_sv(local_arena->arena, &err, base_dir);
            string_append_cstr(local_arena->arena, &err, " failed with line: ");
            string_append_sv(local_arena->arena, &err, line);
            printf("\n=====\n");
            string_print(&err);
            printf("\n=====\n");
            *((char*)0) = 0;
        }

        line = sv_chop_by_delim_sv(&file, new_line);
    }

    local_arena_alloc_reset(local_arena);
    return NULL;
}

Model* model_convert_from_obj(Arena* arena, ObjModel* obj_model) {
    return (Model*)obj_model;
}

Model* model_create_default_model(Arena* arena) {
    return NULL;
}

Model* model_read(Arena* arena, StringView filepath) {
    Model* model = NULL;

    StringView ext = sv_file_extension(filepath);

    if (sv_equal(ext, sv_from_cstr(".obj"))) {
        String file = read_complete_file(arena, filepath);
        StringView file_content = sv_from_string(file);
        StringView base_dir = sv_directory_name(filepath);
        ObjModel* obj_model = model_parse_obj(arena, file_content, base_dir);
        model = model_convert_from_obj(arena, obj_model);
    }
    // else if (sv_equal(ext, sv_from_cstr(".gltf"))) {
    //     // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
    // }
    else {
        model = model_create_default_model(arena);
    }

    return model;
}

