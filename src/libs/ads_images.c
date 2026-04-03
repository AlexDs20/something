#include <stdio.h>

#include "libs/ads_images.h"
#include "platform/io.h"
#include "libs/ads_jpeg.h"

#ifdef ADS_USE_EXTERNAL
#define STB_IMAGE_IMPLEMENTATION
#include "extern/stb_image.h"
#endif

void create_missing_image(Arena* arena, Image* out) {
    out->width  = 128;
    out->height = 128;
    out->data = (u8*)arena_alloc_push(arena, out->width*out->height*sizeof(u32));

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

Image image_read_file(Arena* persist_arena, StringView filename) {
    Image out;
    LocalArena* local_arena = local_arena_alloc_create();
    u64 checkpoint = arena_alloc_checkpoint(persist_arena);
    ImageParsingResult result = {IMAGE_FAIL, 0};

#ifdef ADS_USE_EXTERNAL
    int w, h, c, ok;
    const char* f = sv_as_cstr(local_arena->arena, filename);
    ok = stbi_info(f, &w, &h, &c);
    if (ok == 1) {
        if (c == 1) {
            out.data = stbi_load(f, &w, &h, &c, 0);
        }
        else {
            out.data = stbi_load(f, &w, &h, &c, 4);
        }
        out.width = w;
        out.height = h;
        out.components = c;

        if (out.data != NULL) {
            result.status = IMAGE_SUCCESS;
        }
    }
#else
    String data_str = read_complete_file(local_arena->arena, filename);
    StringView data = sv_from_string(data_str);
    StringView extension = sv_file_extension(filename);

    if (sv_equal(extension, sv_from_cstr(".jpg")) || sv_equal(extension, sv_from_cstr(".jpeg"))) {
        result = decode_jpeg(persist_arena, data, &out);
    }
    else {
        NOT_IMPLEMENTED("Image decoding other than jpeg");
    }
#endif

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
