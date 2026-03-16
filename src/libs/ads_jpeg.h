#ifndef ADS_JPEG_H
#define ADS_JPEG_H

#include "libs/ads_string.h"
#include "libs/ads_images.h"
#include "utils/types.h"
#include "platform/io.h"
#include "math.h"

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

// Forward declare
int read_jpeg_info(StringView filename, u16* width, u16* height, u8* components, u8* precision);

ImageParsingResult decode_jpeg(Arena* persist_arena, StringView data, Image* out);
ImageParsingResult decode_jpeg(Arena* persist_arena, StringView data, Image* out, bool flip_vertically);
#endif // _LIBIMAGES_H
