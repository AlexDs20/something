#ifndef ADS_IMAGE_H
#define ADS_IMAGE_H

#include "memory/allocators.h"
#include "libs/libstring.h"

typedef struct Image Image;
struct Image {
    union {
        u32* data;
        u32* buffer;
        u8* gray;
        u32* rgb;
        u32* rgba;
    };
    u32 width;
    u32 height;
    u8 components;
    u8 precision;
};

Image read_image_file(Arena* arena, string8 filename);
// status = read_image_info(filename, &width, &height, &components);
#endif
