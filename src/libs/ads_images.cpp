#include "libs/ads_string.h"
#include "memory/allocators.h"
#include "libs/ads_images.h"
#include "libs/ads_jpeg.h"
#include "platform/io.h"

#define STB_IMAGE_IMPLEMENTATION
#include "extern/stb_image.h"

void create_missing_image(Arena* arena, Image* out) {
    out->width  = 128;
    out->height = 128;
    out->data = (u32*)arena_alloc_push(arena, out->width*out->height*sizeof(u32));

    const u32 pattern_size = 64;
    for (u32 j=0; j<out->height; j++) {
        for (u32 i=0; i<out->width; i++) {
            u32 linear_index = j*out->width + i;
            u32 color = 0;
            if (((u32)j/pattern_size + (u32)i/pattern_size) % 2 == 1) {
                color = 0xFF00FF;
            }
            out->data[linear_index] = color;
        }
    }
    return;
}

// TODO(alex)
/*
bool read_image_info(filename, &width, &height, &components){
    bool success = true;
    *width = 0;
    *height = 0;
    *components = 0;

    LocalArena* local_arena = local_arena_alloc_create();

    StringView data = read_complete_file(local_arena->arena, filename);
    StringView extension = sv_file_extension(filename);

    ImageParsingResult result = {IMAGE_SUCCESS, 0};
    if (sv_equal(extension, sv_from_cstr(".jpg")) || sv_equal(extension, sv_from_cstr(".jpeg")) {
        // result = read_jpeg_info(data, width, height, components);
        result = {IMAGE_FAIL, "Not implemented"};
    }
    else {
        printf("File format not supported: extension = ");
        string_print(extension);
        return false;
    }

    if (result.status != IMAGE_SUCCESS) {
        return false;
    }

    return success;
}
*/

Image read_image_file(Arena* persist_arena, StringView filename) {
    Image out = {};
    LocalArena* local_arena = local_arena_alloc_create();
    u64 checkpoint = arena_alloc_checkpoint(persist_arena);

    String data_str = read_complete_file(local_arena->arena, filename);
    StringView data = sv_from_string(data_str);
    StringView extension = sv_file_extension(filename);

    ImageParsingResult result = {IMAGE_SUCCESS, 0};
    if (sv_equal(extension, sv_from_cstr(".jpg")) || sv_equal(extension, sv_from_cstr(".jpeg"))) {
        // u16 width;
        // u16 height;
        // u8 components;
        // u8 precision;
        // read_jpeg_info(filename, &width, &height, &components, &precision);
        result = decode_jpeg(persist_arena, data, &out);

        // const char* f = sv_as_cstr(local_arena->arena, filename);
        // int w, h, c;
        // out.gray = stbi_load(f, &w, &h, &c, 4);
        // out.width = w;
        // out.height = h;
        // out.components = c;
    } else if (sv_equal(extension, sv_from_cstr(".png"))) {
        const char* f = sv_as_cstr(local_arena->arena, filename);
        int w, h, c;
        out.gray = stbi_load(f, &w, &h, &c, 0);
        out.width = w;
        out.height = h;
        out.components = c;

        // TODO(alex): Own png decoder
        // decode_png(persist_arena, data, &out);
    } else {
        printf("File format not supported: extension = ");
        sv_print(extension);
    }

    if (result.status != IMAGE_SUCCESS) {
        arena_alloc_restore_zero(persist_arena, checkpoint);
        printf("Failed parsing file: ");
        sv_print(filename);
        printf(" with error: %s\n", result.error_message);
        create_missing_image(persist_arena, &out);
    }

    local_arena_alloc_reset(local_arena);
    return out;
}
