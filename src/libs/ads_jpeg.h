#ifndef ADS_JPEG_H
#define ADS_JPEG_H

#include "libs/libstring.h"
#include "libs/ads_images.h"
#include "libs/ads_math.h"
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
    // TODO(alex): Make this string8?
    const char* error_message;
};

ImageParsingResult decode_jpeg(Arena* persist_arena, string8 data, Image* out);
#endif // _LIBIMAGES_H
