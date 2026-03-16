#ifndef ADS_JPEG_H
#define ADS_JPEG_H

#include <math.h>

#include "libs/ads_string.h"
#include "base/base.h"
#include "libs/ads_images.h"

// Forward declare
int read_jpeg_info(StringView filename, u16* width, u16* height, u8* components, u8* precision);

ImageParsingResult decode_jpeg(Arena* persist_arena, StringView data, Image* out);
ImageParsingResult decode_jpeg(Arena* persist_arena, StringView data, Image* out, bool flip_vertically);
#endif // ADS_JPEG_H
