#ifndef _LIBIMAGES_H
#define _LIBIMAGES_H

#include "libstring.h"
#include "utils/types.h"
#include "platform/io.h"

/*  ============
 *  JPEG
 *  ============
 *  Markers:
 *      FFD8: start of image file
 *      FFD9: end of image file
 *      All except FFD0 to FFD9 and FF01 are followed by length
 */

enum Markers {
    StartOfImage = 0xFFD8,
    ApplicationDefaultHeader = 0xFFE0,
    QuantizationTable = 0xFFDB,
    StartOfFrame = 0xFFC0,
    DefineHuffmanTable = 0xFFC4,
    StartOfScan = 0xFFDA,
    EndOfImage = 0xFFD9,
};

void rgb_to_ycbcr(void) {
}

void ycbcr_to_rgb(void) {
}

void DCT(void) {
}

void IDCT(void) {
}

void read_header();
void write_header();

u32* decode_jpeg(void* data, u64 size) {
    u32* out = 0;
    u64 pos = 0;
    u16 marker;
    u16 length;
    while (true) {
        marker = *((u16*)((u8*)data + pos));
        pos += 2;
        length = 0;
        if (marker == StartOfImage) {
            // length = *((u16*)((u8*)data + pos));
        } else if (marker == EndOfImage) {
            break;
        } else if (marker == StartOfScan) {

        }

        printf("marker = %d, length = %d\n", marker, length);
    }
    return out;
}

u32* read_image_file(Arena* arena, string8 filename) {
    LocalArena* local_arena = local_arena_alloc_create();

    u32* out = 0;
    string8 data = read_file(local_arena->arena, filename);

    string8 extension = string_get_file_extension(filename);
    if (extension == ".jpg" || extension == ".jpeg") {
        printf("Reading .jpeg ");
        string_print(filename);
        printf("\n");
        u32* rgb = decode_jpeg((void*)data.buffer, data.size);
    } else if (extension == ".png"){
        printf("Reading .png ");
        string_print(filename);
        printf("\n");
        // u32* rgb = decode_png(data);
    } else {
        printf("File format not supported: extension = ");
        string_print(extension);
        printf("for file: ");
        string_print(filename);
        printf("\n");
    }

    local_arena_alloc_reset(local_arena);
    return out;
}

#endif // _LIBIMAGES_H
