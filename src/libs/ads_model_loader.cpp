#include "libs/ads_model_loader.h"
#include <stdio.h>

// TODO remove from here
#define GiB (1024*1024*1024)    // GiB
#define MiB (1024*1024)         // MiB

static Vector* read_mtl_file(Arena* arena, Arena* string_arena, StringView filepath) {
    /*
     * Sources:
     * https://en.wikipedia.org/wiki/Wavefront_.obj_file
     * https://www.fileformat.info/format/wavefrontobj/egff.htm
     * https://paulbourke.net/dataformats/mtl/
     */
    LocalArena* local_arena = local_arena_alloc_create();

    String content = read_complete_file(local_arena->arena, filepath);
    StringView file = sv_from_string(content);

    ObjMaterial dummy_mats = {0};
    Vector* materials = (Vector*)vector_alloc_create(arena, sizeof(ObjMaterial));

    ObjMaterial* current_mats = NULL;

    StringView space    = sv_from_cstr(" ");
    StringView new_line = sv_from_cstr("\n");
    StringView newmtl   = sv_from_cstr("newmtl ");

    StringView line = sv_trim_front(sv_trim_back(sv_chop_by_delim_sv(&file, new_line)));
    while (file.size != 0) {
        if (sv_starts_with(line, newmtl)) {
            line = sv_truncate_front(line, newmtl.size);

            current_mats = (ObjMaterial*)vector_alloc_push(materials, &dummy_mats);
            current_mats->name = sv_from_string(
                    string_init_sv(string_arena, sv_trim_front(line))
                    );
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
        else if (sv_starts_with_cstr(line, "Tr ")) {        // Transparency
            line = sv_truncate_front(line, 3);
            sv_parse_f32(&line, &current_mats->Tr);
        }
        else if (sv_starts_with_cstr(line, "d ")) {         // dissolve: Tr = 1-d
            line = sv_truncate_front(line, 2);
            float d;
            sv_parse_f32(&line, &d);
            current_mats->Tr = 1.0f-d;
        }
        else if (sv_starts_with_cstr(line, "illum ")) {     // Illumination
            line = sv_truncate_front(line, 6);
            sv_parse_u32(&line, &current_mats->illum);
        }
        else if (sv_starts_with_cstr(line, "Tf ")) {        // Transmission filter color
            line = sv_truncate_front(line, 3);
            sv_parse_f32(&line, &current_mats->Tf);
        }
        // else if (sv_starts_with_cstr(line, "sharpness ")) {
        // }
        else if (sv_starts_with_cstr(line, "map_Ka ")) {    // texture map ambient color
            line = sv_truncate_front(line, 7);
            current_mats->map_Ka = sv_from_string(
                                    string_init_sv(string_arena, sv_trim_front(line))
                                    );
        }
        else if (sv_starts_with_cstr(line, "map_Kd ")) {    // texture map diffuse color
            line = sv_truncate_front(line, 7);
            current_mats->map_Kd = sv_from_string(
                                    string_init_sv(string_arena, sv_trim_front(line))
                                    );
        }
        else if (sv_starts_with_cstr(line, "map_Ks ")) {    // texture map specular color
            line = sv_truncate_front(line, 7);
            current_mats->map_Ks = sv_from_string(
                                    string_init_sv(string_arena, sv_trim_front(line))
                                    );
        }
        // else if (sv_starts_with_cstr(line, "map_Ns ")) {    // specular highlight component
        // }
        else if (sv_starts_with_cstr(line, "map_d ")) {     // alpha texture map
            line = sv_truncate_front(line, 6);
            current_mats->map_d = sv_from_string(
                                    string_init_sv(string_arena, sv_trim_front(line))
                                    );
        }
        else if (sv_starts_with_cstr(line, "map_bump ")) {  // map_bump and bump: same
            line = sv_truncate_front(line, 9);
            current_mats->map_Bump = sv_from_string(
                                    string_init_sv(string_arena, sv_trim_front(line))
                                    );
        }
        else if (sv_starts_with_cstr(line, "map_Bump ")) {
            line = sv_truncate_front(line, 9);
            current_mats->map_Bump = sv_from_string(
                                    string_init_sv(string_arena, sv_trim_front(line))
                                    );
        }
        else if (sv_starts_with_cstr(line, "bump ")) {
            line = sv_truncate_front(line, 5);
            current_mats->map_Bump = sv_from_string(
                                    string_init_sv(string_arena, sv_trim_front(line))
                                    );        }
        // else if (sv_starts_with_cstr(line, "disp ")) {      // displacement map
        // }
        // else if (sv_starts_with_cstr(line, "decal ")) {     // stencial decal texture
        // }
        // else if (sv_starts_with_cstr(line, "refl ")) {      // reflection map
        // }
        else if (sv_starts_with_cstr(line, "#")) {
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

        line = sv_trim_front(sv_trim_back(sv_chop_by_delim_sv(&file, new_line)));
    }

    local_arena_alloc_reset(local_arena);
    return materials;
}

ObjModel* model_parse_obj(Arena* persist_arena, StringView file, StringView base_dir) {
// #define SINGLEPASS
// #define RAWBUFFER
#if !defined(SINGLEPASS)
    Arena* string_arena = arena_alloc_create(1*MiB);
    LocalArena* local_arena = local_arena_alloc_create();

    // First pass
    StringView new_line = sv_from_cstr("\n");
    StringView usemtl = sv_from_cstr("usemtl");
    StringView mtllib = sv_from_cstr("mtllib");

    StringView file_copy = file;

    // First pass counting the amount of vertex etc
    uint32_t n_v = 0, n_vt = 0, n_vn = 0, n_f = 0, n_g = 0;
    while (file.size != 0) {
        file = sv_trim_front(file);
        // Hacking it
        if (file.buffer[0] == 'v') {
            switch (file.buffer[1]) {
                case ' ': {
                    n_v++;
                } break;
                case 't': {
                    n_vt++;
                } break;
                case 'n': {
                    n_vn++;
                } break;
            }
        }
        else if (file.buffer[0] == 'f') {
                    n_f++;
        }
        else if (file.buffer[0] == 'g' || file.buffer[0] == 'o') {
                    n_g++;
        }
        sv_chop_by_delim_sv(&file, new_line);
    }
    // If no group => create a default one
    if (n_g == 0) n_g++;
    file = file_copy;

    // Second pass => the actual parsing
    ObjModel* obj_model = (ObjModel*)arena_alloc_push_zero(persist_arena, sizeof(ObjModel));

    Vec3f* vec_vertex    = (Vec3f*)   arena_alloc_push_zero(persist_arena, sizeof(Vec3f)   * n_v);
    Vec3f* vec_texcoords = (Vec3f*)   arena_alloc_push_zero(persist_arena, sizeof(Vec3f)   * n_vt);
    Vec3f* vec_normals   = (Vec3f*)   arena_alloc_push_zero(persist_arena, sizeof(Vec3f)   * n_vn);
    ObjFace* vec_faces   = (ObjFace*) arena_alloc_push_zero(persist_arena, sizeof(ObjFace) * n_f);
    ObjGroup* vec_groups = (ObjGroup*)arena_alloc_push_zero(persist_arena, sizeof(ObjGroup)* n_g);
    uint32_t i_v = 0, i_vt = 0, i_vn = 0, i_f = 0, i_g = 0;

    Vec3f* current_vertex   = vec_vertex;
    Vec3f* current_texcoord = vec_texcoords;
    Vec3f* current_normal   = vec_normals;
    ObjFace* current_face   = vec_faces;
    ObjGroup* current_group = vec_groups;

    // Default group in case nothing is given in the obj file
    *current_group = {0};
    current_group->name = string_init_cstr(string_arena, "default");
    int current_shading_group = 0;

    // TODO: Handle the materials properly by also creating a default material
    Arena* materials_arena = arena_alloc_create(1*GiB);
    Vector* vec_materials = vector_alloc_create(materials_arena, sizeof(ObjMaterial));
    int current_material_index = 0;

    StringView line = sv_trim_front(sv_trim_back(sv_chop_by_delim_sv(&file, new_line)));
    while (file.size != 0) {
        if (sv_starts_with_char(line, 'v')) {
            Vec3f* v = NULL;
            int r;

            sv_chop_by(&line, 1);
            if (sv_starts_with_char(line, ' ')) {       // vertex
                line = sv_truncate_front(line, 2);
                v = current_vertex++;
                i_v++;
            }
            else if (sv_starts_with_char(line, 't')) {  // texture coord
                line = sv_truncate_front(line, 3);
                v = current_texcoord++;
                i_vt++;
            }
            else if (sv_starts_with_char(line, 'n')) {  // vertex normal
                line = sv_truncate_front(line, 3);
                v = current_normal++;
                i_vn++;
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
            // TODO:
            //  - Add support for n-gons?
            line = sv_truncate_front(line, 2);
            int r;

            if (vec_groups->face_count++ == 0) {
                vec_groups->first_face_index = i_f;
            }

            // Actually parse the face
            ObjFace* f = current_face++;
            i_f++;
            f->material_index = current_material_index;
            f->shading_group = current_shading_group;

            StringView delim = sv_from_cstr("/");
            StringView sep;

            // Parse indices and convert to point to correct elements
            r = sv_parse_u32(&line, &f->v_indices[0]);
            f->v_indices[0] = f->v_indices[0] >= 0 ? f->v_indices[0]-- : f->v_indices[0] + i_v + 1;

            sep = sv_chop_by(&line, 1);                 // sep could be '/' or ' '. if '/' => read vt and vn
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &f->vt_indices[0]);
                f->vt_indices[0] = f->vt_indices[0] >= 0 ? f->vt_indices[0]-- : f->vt_indices[0] + i_vt + 1;

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &f->vn_indices[0]);
                    f->vn_indices[0] = f->vn_indices[0] >= 0 ? f->vn_indices[0]-- : f->vn_indices[0] + i_vn + 1;
                }
            }

            r = sv_parse_u32(&line, &f->v_indices[1]);
            f->v_indices[1] = f->v_indices[1] >= 0 ? f->v_indices[1]-- : f->v_indices[1] + i_v + 1;

            sep = sv_chop_by(&line, 1);
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &f->vt_indices[1]);
                f->vt_indices[1] = f->vt_indices[1] >= 0 ? f->vt_indices[1]-- : f->vt_indices[1] + i_vt + 1;

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &f->vn_indices[1]);
                    f->vn_indices[1] = f->vn_indices[1] >= 0 ? f->vn_indices[1]-- : f->vn_indices[1] + i_vn + 1;
                }
            }

            r = sv_parse_u32(&line, &f->v_indices[2]);
            f->v_indices[2] = f->v_indices[2] >= 0 ? f->v_indices[2]-- : f->v_indices[2] + i_v + 1;
            sep = sv_chop_by(&line, 1);
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &f->vt_indices[2]);
                f->vt_indices[2] = f->vt_indices[2] >= 0 ? f->vt_indices[2]-- : f->vt_indices[2] + i_vt + 1;

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &f->vn_indices[2]);
                    f->vn_indices[2] = f->vn_indices[2] >= 0 ? f->vn_indices[2]-- : f->vn_indices[2] + i_vn + 1;
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
            int found = 0;
            for (int i=0; i<vec_materials->count; i++) {
                ObjMaterial* m = (ObjMaterial*)vector_alloc_get(vec_materials, i);
                if (sv_equal(line, m->name)) {
                    found = 1;
                    current_material_index = i;
                    break;
                }
            }
            // If I had a logger, log that we did not find the material
            //  => use "default"
            // if (!found) {
            //     *(volatile char*) 0 = 0;
            // }
        }
        else if (sv_starts_with(line, mtllib)) {
            line = sv_truncate_front(line, mtllib.size+1);

            String fp = string_init_sv(string_arena, base_dir);
            string_append_sv(string_arena, &fp, line);
            StringView mtl_file = sv_from_string(fp);

            vec_materials = read_mtl_file(materials_arena, string_arena, mtl_file);
            obj_model->mtllib_name = sv_from_string(fp);
        }
        else if (sv_starts_with_char(line, 'o') || sv_starts_with_char(line, 'g')) {      // Object or group
            StringView group_name = sv_truncate_front(line, 2);

            ObjGroup* g = current_group++;
            i_g++;
            g->name = string_init_sv(string_arena, group_name);
            g->material_index = current_material_index;
            g->face_count = 0;
            g->first_face_index = 0;
        }
        else if (sv_starts_with_char(line, '#')) {
        }
        else if (sv_trim_front(line).size == 0) {           // If there are only spaces
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

        line = sv_trim_front(sv_trim_back(sv_chop_by_delim_sv(&file, new_line)));
    }

    // Copy data to persistent arena and to the obj_model variable.
    obj_model->materials = vector_alloc_copy_to_arena(persist_arena, vec_materials);

    // Note we also need copy the strings for the materials to the persistent arena
    for (int i=0; i<vec_materials->count; i++) {
        int r;
        ObjMaterial* tmp_m = (ObjMaterial*)vector_alloc_get(vec_materials, i);
        ObjMaterial* new_m = (ObjMaterial*)vector_alloc_get(obj_model->materials, i);
    }

    arena_alloc_free(string_arena);
    arena_alloc_free(materials_arena);

    local_arena_alloc_reset(local_arena);
    return obj_model;
#else
#endif
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

