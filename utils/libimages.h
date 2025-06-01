#ifndef _LIBIMAGES_H
#define _LIBIMAGES_H

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

void parse_jpeg(void* data) {
    u64 pos = 0;
    u16 marker;
    u16 length;
    while (true) {
        marker = *((u16*)((u8*)data + pos));
        pos += 2;
        length = 0;
        if (marker == StartOfImage) {
            length = *((u16*)((u8*)data + pos));
        } else if (marker == EndOfImage) {
            break;
        } else if (marker == StartOfScan) {

        }

        printf("marker = %d, length = %d\n", marker, length);
    }
}

#endif // _LIBIMAGES_H
