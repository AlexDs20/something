#include "libs/libstring.h"
#include "libs/ads_jpeg.h"

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

    string8 data = read_file(local_arena->arena, filename);
    string8 extension = string_get_file_extension(filename);

    ImageParsingResult result = {IMAGE_SUCCESS, 0};
    if (extension == ".jpg" || extension == ".jpeg") {
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

Image read_image_file(Arena* persist_arena, string8 filename) {
    Image out = {};
    LocalArena* local_arena = local_arena_alloc_create();
    u64 checkpoint = arena_alloc_checkpoint(persist_arena);

    string_print(filename);
    string8 data = read_file(local_arena->arena, filename);
    string8 extension = string_get_file_extension(filename);

    ImageParsingResult result = {IMAGE_SUCCESS, 0};
    if (extension == ".jpg" || extension == ".jpeg") {

        u16 width;
        u16 height;
        u8 components;
        u8 precision;
        read_jpeg_info(filename, &width, &height, &components, &precision);

        printf("width: %d, height: %d, components: %d, precision: %d\n", width, height, components, precision);

        // char* f = string_to_cstr(local_arena->arena, filename);
        // int w, h, c;
        // out.gray = stbi_load(f, &w, &h, &c, 4);
        // out.width = w;
        // out.height = h;
        // out.components = c;
        // printf("%d %d %d %d\n", out.gray[0], out.gray[1], out.gray[2], out.gray[3]);
        // // NOTE(alex): slightly off for the current gray image (only by one, check clamp, ...)
        // TODO(alex): Change the API to this
        // u8* buffer = nullptr;
        // u32 w;
        // u32 h;
        // u8 c;
        // success = decode_jpeg(data, buffer, &w, &h, &c);
        result = decode_jpeg(persist_arena, data, &out);
        // printf("%d %d %d %d\n", out.gray[0], out.gray[1], out.gray[2], out.gray[3]);
    } else if (extension == ".png"){
        char* f = string_to_cstr(local_arena->arena, filename);
        int w, h, c;
        // out.gray = stbi_load(f, &w, &h, &c, 0);
        out.width = w;
        out.height = h;
        out.components = c;
        // TODO(alex): Own png decoder
        // decode_png(persist_arena, data, &out);
    } else {
        printf("File format not supported: extension = ");
        string_print(extension);
    }

    if (result.status != IMAGE_SUCCESS) {
        arena_alloc_restore_zero(persist_arena, checkpoint);
        printf("Failed parsing file: ");
        string_print(filename);
        printf(" with error: %s\n", result.error_message);
        create_missing_image(persist_arena, &out);
    }

    local_arena_alloc_reset(local_arena);
    return out;
}
