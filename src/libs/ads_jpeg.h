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


int read_jpeg_info(string8 filename, u16* width, u16* height, u8* components, u8* precision);

ImageParsingResult decode_jpeg(Arena* persist_arena, string8 data, Image* out);
ImageParsingResult decode_jpeg(Arena* persist_arena, string8 data, Image* out, bool flip_vertically);
#endif // _LIBIMAGES_H
