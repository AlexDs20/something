#include "libs/ads_model_loader.h"
#include <stdio.h>


static void read_mtl_file(StringView filepath) {
    /*
     * Sources:
     * https://en.wikipedia.org/wiki/Wavefront_.obj_file
     * https://www.fileformat.info/format/wavefrontobj/egff.htm
     * https://paulbourke.net/dataformats/mtl/
     */
    LocalArena* local_arena = local_arena_alloc_create();
    String content = read_complete_file(local_arena->arena, filepath);
    StringView file = sv_from_string(content);

    StringView space = sv_from_cstr(" ");
    StringView new_line = sv_from_cstr("\n");
    StringView newmtl = sv_from_cstr("newmtl");
    StringView line = sv_chop_by_delim_sv(&file, new_line);
    while (file.size != 0) {
        if (sv_starts_with(line, newmtl)) {
            line = sv_truncate_front(line, newmtl.size+1);
            // sv_print(line);
        }
        else if (sv_starts_with_cstr(line, "Ka ")) {        // ambient color
            line = sv_truncate_front(line, 3);
            float r=0.0f, g=0.0f, b=0.0f;
            sv_parse_f32(&line, &r);
            sv_parse_f32(&line, &g);
            sv_parse_f32(&line, &b);
            printf("\nc=(%f,%f,%f)", r, g, b);
        }
        else if (sv_starts_with_cstr(line, "Kd ")) {        // diffuse color
            line = sv_truncate_front(line, 3);
            float r=0.0f, g=0.0f, b=0.0f;
            sv_parse_f32(&line, &r);
            sv_parse_f32(&line, &g);
            sv_parse_f32(&line, &b);
            printf("\nc=(%f,%f,%f)", r, g, b);
        }
        else if (sv_starts_with_cstr(line, "Ks ")) {        // specular color
            line = sv_truncate_front(line, 3);
            float r=0.0f, g=0.0f, b=0.0f;
            sv_parse_f32(&line, &r);
            sv_parse_f32(&line, &g);
            sv_parse_f32(&line, &b);
            printf("\nc=(%f,%f,%f)", r, g, b);
        }
        else if (sv_starts_with_cstr(line, "Ke ")) {        // emissive color
            line = sv_truncate_front(line, 3);
            float r=0.0f, g=0.0f, b=0.0f;
            sv_parse_f32(&line, &r);
            sv_parse_f32(&line, &g);
            sv_parse_f32(&line, &b);
            printf("\nc=(%f,%f,%f)", r, g, b);
        }
        else if (sv_starts_with_cstr(line, "Ns ")) {        // specular exponent
            line = sv_truncate_front(line, 3);
            float Ns=0.0f;
            sv_parse_f32(&line, &Ns);
        }
        else if (sv_starts_with_cstr(line, "Ni ")) {        // Index of refraction
            line = sv_truncate_front(line, 3);
            float Ni=0.0f;
            sv_parse_f32(&line, &Ni);
        }
        // else if (sv_starts_with_cstr(line, "Tr ")) {        // Transparency
        // }
        else if (sv_starts_with_cstr(line, "d ")) {         // dissolve: Tr = 1-d
            line = sv_truncate_front(line, 3);
            float d=0.0f;
            sv_parse_f32(&line, &d);
        }
        else if (sv_starts_with_cstr(line, "illum ")) {     // Illumination
            line = sv_truncate_front(line, 3);
            uint32_t illum=0.0f;
            sv_parse_u32(&line, &illum);
        }
        // else if (sv_starts_with_cstr(line, "Tf ")) {        // Transmission filter color
        // }
        // else if (sv_starts_with_cstr(line, "sharpness ")) {
        // }
        // else if (sv_starts_with_cstr(line, "map_Ka ")) {    // texture map ambient color
        // }
        else if (sv_starts_with_cstr(line, "map_Kd ")) {    // texture map diffuse color
            line = sv_truncate_front(line, 7);
        }
        else if (sv_starts_with_cstr(line, "map_Ks ")) {    // texture map specular color
            line = sv_truncate_front(line, 7);
        }
        // else if (sv_starts_with_cstr(line, "map_Ns ")) {    // specular highlight component
        // }
        // else if (sv_starts_with_cstr(line, "map_d ")) {     // alpha texture map
        // }
        else if (sv_starts_with_cstr(line, "map_bump ")) {  // map_bump and bump: same
            line = sv_truncate_front(line, 9);
        }
        else if (sv_starts_with_cstr(line, "map_Bump ")) {
            line = sv_truncate_front(line, 9);
        }
        else if (sv_starts_with_cstr(line, "bump ")) {
            line = sv_truncate_front(line, 9);
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
    return;
}

ObjModel* model_parse_obj(Arena* arena, StringView file, StringView base_dir) {
    LocalArena* local_arena = local_arena_alloc_create();

    // Setup the different types
    StringView new_line = sv_from_cstr("\n");
    StringView usemtl = sv_from_cstr("usemtl");
    StringView mtllib = sv_from_cstr("mtllib");

    StringView line = sv_chop_by_delim_sv(&file, new_line);
    while (file.size != 0) {
        if (sv_starts_with_char(line, 'v')) {
            sv_chop_by(&line, 1);
            if (sv_starts_with_char(line, ' ')) {       // vertex
                line = sv_truncate_front(line, 2);
                float x=0, y=0, z=0;
                int r;
                r = sv_parse_f32(&line, &x);
                r = sv_parse_f32(&line, &y);
                r = sv_parse_f32(&line, &z);
            }
            else if (sv_starts_with_char(line, 't')) {  // texture coord
                line = sv_truncate_front(line, 3);
                float u=0, v=0, w=0;
                int r;
                r = sv_parse_f32(&line, &u);
                r = sv_parse_f32(&line, &v);
                r = sv_parse_f32(&line, &w);
            }
            else if (sv_starts_with_char(line, 'n')) {  // vertex normal
                line = sv_truncate_front(line, 3);
                float nx=0, ny=0, nz=0;
                int r;
                r = sv_parse_f32(&line, &nx);
                r = sv_parse_f32(&line, &ny);
                r = sv_parse_f32(&line, &nz);
            }
            // else if (sv_starts_with_char(line, 'p')) {  // parameter space vertices
            // }
            else {
                *((char*)0) = 0;
            }
        }
        else if (sv_starts_with_char(line, 'f')) {      // face
            line = sv_truncate_front(line, 2);
            uint32_t v1=0,  v2=0,  v3=0;
            uint32_t vt1=0, vt2=0, vt3=0;
            uint32_t vn1=0, vn2=0, vn3=0;
            int r;

            StringView delim = sv_from_cstr("/");
            StringView sep;

            r = sv_parse_u32(&line, &v1);
            sep = sv_chop_by(&line, 1);                 // sep could be '/' or ' '. if '/' => read vt and vn
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &v2);

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &v3);
                }
            }

            r = sv_parse_u32(&line, &vt1);
            sep = sv_chop_by(&line, 1);
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &vt2);

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &vt3);
                }
            }

            r = sv_parse_u32(&line, &vn1);
            sep = sv_chop_by(&line, 1);
            if (sv_equal(sep, delim)) {
                r = sv_parse_u32(&line, &vn2);

                sep = sv_chop_by(&line, 1);
                if (sv_equal(sep, delim)) {
                    r = sv_parse_u32(&line, &vn3);
                }
            }

            // printf("\nv=(%u,%u,%u) vt=(%u,%u,%u) vn=(%u,%u,%u)", v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3);

        }
        else if (sv_starts_with_char(line, 's')) {      // smooth shading s 1  or s off
            uint32_t s = 0;
            line = sv_truncate_front(line, 2);
            if (sv_starts_with_cstr(line, "0") || sv_starts_with_cstr(line, "off")) {
                // TODO: Set off
            }
            else {
                // TODO Set shading
                int r = sv_parse_u32(&line, &s);
            }
        }
        // else if (sv_starts_with_char(line, 'l')) {      // line element
        // }
        // else if (sv_starts_with_char(line, 'p')) {      // point element
        // }
        else if (sv_starts_with(line, usemtl)) {
            line = sv_truncate_front(line, usemtl.size+1);
            StringView material_name = sv_trim_back(line);
        }
        else if (sv_starts_with(line, mtllib)) {
            line = sv_trim_back(sv_truncate_front(line, mtllib.size+1));

            String fp = string_init_sv(local_arena->arena, base_dir);
            string_append_sv(local_arena->arena, &fp, line);

            StringView mtl_file = sv_from_string(fp);
            read_mtl_file(mtl_file);
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

