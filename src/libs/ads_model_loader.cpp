#include "libs/ads_model_loader.h"
#include <stdio.h>

// TODO remove from here
#define GiB (1024*1024*1024)     // GiB

// static ObjMaterial* read_mtl_file(Arena* arena, StringView filepath, int32_t* material_count) {
static void read_mtl_file(Vector* materials, StringView filepath) {
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

    ObjMaterial* current_mats = NULL;
    String current_mat_name;

    StringView space = sv_from_cstr(" ");
    StringView new_line = sv_from_cstr("\n");
    StringView newmtl = sv_from_cstr("newmtl");

    StringView line = sv_trim_front(sv_trim_back(sv_chop_by_delim_sv(&file, new_line)));
    while (file.size != 0) {
        if (sv_starts_with(line, newmtl)) {
            line = sv_truncate_front(line, newmtl.size+1);

            current_mats = (ObjMaterial*)vector_alloc_push(materials, &dummy_mats);
            current_mats->name = string_init_sv(local_arena->arena, line);
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
            current_mats->map_Ka = string_init_sv(local_arena->arena, line);
        }
        else if (sv_starts_with_cstr(line, "map_Kd ")) {    // texture map diffuse color
            line = sv_truncate_front(line, 7);
            current_mats->map_Kd = string_init_sv(local_arena->arena, line);
        }
        else if (sv_starts_with_cstr(line, "map_Ks ")) {    // texture map specular color
            line = sv_truncate_front(line, 7);
            current_mats->map_Ks = string_init_sv(local_arena->arena, line);
        }
        // else if (sv_starts_with_cstr(line, "map_Ns ")) {    // specular highlight component
        // }
        else if (sv_starts_with_cstr(line, "map_d ")) {     // alpha texture map
            line = sv_truncate_front(line, 6);
            current_mats->map_d = string_init_sv(local_arena->arena, line);
        }
        else if (sv_starts_with_cstr(line, "map_bump ")) {  // map_bump and bump: same
            line = sv_truncate_front(line, 9);
            current_mats->map_Bump = string_init_sv(local_arena->arena, line);
        }
        else if (sv_starts_with_cstr(line, "map_Bump ")) {
            line = sv_truncate_front(line, 9);
            current_mats->map_Bump = string_init_sv(local_arena->arena, line);
        }
        else if (sv_starts_with_cstr(line, "bump ")) {
            line = sv_truncate_front(line, 5);
            current_mats->map_Bump = string_init_sv(local_arena->arena, line);
        }
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

    // Put the strings at the end of the provided arena
    // Make sure each materials point to the right thing after being moved
    int r;
    for (int i=0; i<materials->count; i++) {
        ObjMaterial* m = (ObjMaterial*)vector_alloc_get(materials, i);
        r = string_shrink_to_fit(NULL, &m->name);
        m->name = string_deep_copy(materials->arena, m->name);

        r = string_shrink_to_fit(NULL, &m->map_Ka);
        m->map_Ka = string_deep_copy(materials->arena, m->map_Ka);

        r = string_shrink_to_fit(NULL, &m->map_Kd);
        m->map_Kd = string_deep_copy(materials->arena, m->map_Kd);

        r = string_shrink_to_fit(NULL, &m->map_Ks);
        m->map_Ks = string_deep_copy(materials->arena, m->map_Ks);

        r = string_shrink_to_fit(NULL, &m->map_Bump);
        m->map_Bump = string_deep_copy(materials->arena, m->map_Bump);

        r = string_shrink_to_fit(NULL, &m->map_d);
        m->map_d = string_deep_copy(materials->arena, m->map_d);
    }

    local_arena_alloc_reset(local_arena);
    return;
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
    Vector* vec_materials = vector_alloc_create(materials_arena, sizeof(ObjMaterial));

    Arena* groups_arena = arena_alloc_create(1*GiB);
    Vector* vec_groups = vector_alloc_create(groups_arena, sizeof(ObjGroup));

    ObjModel* obj_model = (ObjModel*)arena_alloc_push(persist_arena, sizeof(ObjModel));

    // Dummy fields / vars useful while parsing
    Vec3f nil_Vec3f = {0};
    ObjFace nil_face = {0};
    ObjGroup nil_group = {0};

    int current_material_index = -1;

    ObjGroup* current_group = NULL;
    int current_shading_group = 0;

    int current_face_index = -1;

    // default group in case nothing is given in the obj file
    current_group = (ObjGroup*)vector_alloc_push(vec_groups, &nil_group);
    current_group->name = string_init_cstr(groups_arena, "default");
    current_group->material_index = -1;
    current_group->face_count = 0;

    // Setup the different types
    StringView new_line = sv_from_cstr("\n");
    StringView usemtl = sv_from_cstr("usemtl");
    StringView mtllib = sv_from_cstr("mtllib");

    StringView line = sv_trim_front(sv_trim_back(sv_chop_by_delim_sv(&file, new_line)));
    while (file.size != 0) {
        if (sv_starts_with_char(line, 'v')) {
            Vec3f* v = NULL;
            int r;

            sv_chop_by(&line, 1);
            if (sv_starts_with_char(line, ' ')) {       // vertex
                line = sv_truncate_front(line, 2);
                v = (Vec3f*)vector_alloc_push(vec_vertex, &nil_Vec3f);
            }
            else if (sv_starts_with_char(line, 't')) {  // texture coord
                line = sv_truncate_front(line, 3);
                v = (Vec3f*)vector_alloc_push(vec_texcoords, &nil_Vec3f);
            }
            else if (sv_starts_with_char(line, 'n')) {  // vertex normal
                line = sv_truncate_front(line, 3);
                v = (Vec3f*)vector_alloc_push(vec_normals, &nil_Vec3f);
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
            //  - Add support for negative relative index
            //  - Add support for n-gons
            line = sv_truncate_front(line, 2);
            int r;

            current_face_index++;
            if (current_group->face_count == 0) {
                current_group->first_face_index = current_face_index;
            }
            current_group->face_count++;

            // Actually parse the face
            ObjFace* f = (ObjFace*)vector_alloc_push(vec_faces, &nil_face);
            f->material_index = current_material_index;
            f->shading_group = current_shading_group;

            StringView delim = sv_from_cstr("/");
            StringView sep;

            // NOTE: No plan on supporting negative indices
            r = sv_parse_u32(&line, &f->v_indices[0]);
            f->v_indices[0]--;
            sep = sv_chop_by(&line, 1);                 // sep could be '/' or ' '. if '/' => read vt and vn
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &f->vt_indices[0]);
                f->vt_indices[0]--;
                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &f->vn_indices[0]);
                    f->vn_indices[0]--;
                }
            }

            r = sv_parse_u32(&line, &f->v_indices[1]);
            f->v_indices[1]--;
            sep = sv_chop_by(&line, 1);
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &f->vt_indices[1]);
                f->vt_indices[1]--;

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &f->vn_indices[1]);
                    f->vn_indices[1]--;
                }
            }

            r = sv_parse_u32(&line, &f->v_indices[2]);
            f->v_indices[2]--;
            sep = sv_chop_by(&line, 1);
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &f->vt_indices[2]);
                f->vt_indices[2]--;

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &f->vn_indices[2]);
                    f->vn_indices[2]--;
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
            for (int i=0; i<vec_materials->count; i++) {
                ObjMaterial* m = (ObjMaterial*)vector_alloc_get(vec_materials, i);
                if (sv_equal(line, sv_from_string(m->name))) {
                    current_material_index = i;
                    break;
                }
            }
        }
        else if (sv_starts_with(line, mtllib)) {
            line = sv_truncate_front(line, mtllib.size+1);

            String fp = string_init_sv(persist_arena, base_dir);
            string_append_sv(persist_arena, &fp, line);
            StringView mtl_file = sv_from_string(fp);

            read_mtl_file(vec_materials, mtl_file);
            obj_model->mtllib_name = fp;
        }
        else if (sv_starts_with_char(line, 'o')) {      // Object name
            StringView object_name = sv_truncate_front(line, 2);

            current_group = (ObjGroup*)vector_alloc_push(vec_groups, &nil_group);
            current_group->name = string_init_sv(persist_arena, object_name);
            current_group->material_index = current_material_index;
            current_group->face_count = 0;
        }
        else if (sv_starts_with_char(line, 'g')) {      // Group name (there can be many, in that case => data after belong to each group)
            StringView group_name = sv_truncate_front(line, 2);

            current_group = (ObjGroup*)vector_alloc_push(vec_groups, &nil_group);
            current_group->name = string_init_sv(persist_arena, group_name);
            current_group->material_index = current_material_index;
            current_group->face_count = 0;
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
    obj_model->vertices  = vector_alloc_copy_to_arena(persist_arena, vec_vertex);
    obj_model->texcoords = vector_alloc_copy_to_arena(persist_arena, vec_texcoords);
    obj_model->normals   = vector_alloc_copy_to_arena(persist_arena, vec_normals);
    obj_model->faces     = vector_alloc_copy_to_arena(persist_arena, vec_faces);
    obj_model->groups    = vector_alloc_copy_to_arena(persist_arena, vec_groups);
    obj_model->materials = vector_alloc_copy_to_arena(persist_arena, vec_materials);

    // Arena* groups_arena = arena_alloc_create(1*GiB);

    // Note we also need copy the strings for the materials to the persistent arena
    for (int i=0; i<vec_materials->count; i++) {
        int r;
        ObjMaterial* tmp_m = (ObjMaterial*)vector_alloc_get(vec_materials, i);
        ObjMaterial* new_m = (ObjMaterial*)vector_alloc_get(obj_model->materials, i);

        new_m->name = string_deep_copy(persist_arena, tmp_m->name);
        new_m->map_Ka = string_deep_copy(persist_arena, tmp_m->map_Ka);
        new_m->map_Kd = string_deep_copy(persist_arena, tmp_m->map_Kd);
        new_m->map_Ks = string_deep_copy(persist_arena, tmp_m->map_Ks);
        new_m->map_Bump = string_deep_copy(persist_arena, tmp_m->map_Bump);
        new_m->map_d = string_deep_copy(persist_arena, tmp_m->map_d);
    }

    arena_alloc_free(vertex_arena);
    arena_alloc_free(texcoords_arena);
    arena_alloc_free(normals_arena);
    arena_alloc_free(faces_arena);
    arena_alloc_free(materials_arena);
    arena_alloc_free(groups_arena);

    local_arena_alloc_reset(local_arena);
    return obj_model;
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

