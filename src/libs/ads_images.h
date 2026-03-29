#ifndef ADS_IMAGE_H
#define ADS_IMAGE_H

#include "libs/ads_string.h"
#include "base/base.h"

#include "memory/allocators.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Image Image;
struct Image {
    u8* data;
    u32 width;
    u32 height;
    u8 components;
    u8 precision;
};

// TODO(alex): Have real errors
typedef enum {
    IMAGE_SUCCESS = 0,
    IMAGE_FAIL,
} ImageError;

typedef struct ImageParsingResult ImageParsingResult;
struct ImageParsingResult {
    ImageError status;
    const char* error_message;
};

// /* bs*, jpeg*? as void* */void read_image_info(filename, &width, &height, &components, &precision);
// void read_image_file(filename, &width, &height, &components);

Image read_image_file(Arena* arena, StringView filename);
// bool read_image_info(filename, &width, &height, &components);
// image_flip_vertically(Image* image);

#ifdef __cplusplus
}
#endif

#endif
