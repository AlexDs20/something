#ifndef _LIBIMAGES_H
#define _LIBIMAGES_H

#include "libstring.h"
#include "utils/types.h"
#include "platform/io.h"

Image read_image_file(Arena* arena, string8 filename);
void write_image_file(u32* data, u64 width, u64 height, string8 filename);

#endif // _LIBIMAGES_H

#ifdef LIB_IMAGES_IMPLEMENTATION
/*  ============
 *  JPEG JFIF
 *  ============
 *  This implementation only convers Baseline Sequential jpg compression (JFIF implementation)
 *  If JPG progressive or gierarchical compression, see itu-1150 standard
 *
 *  - This JPG implementation standard: https://www.w3.org/Graphics/JPEG/itu-t81.pdf
 *  - JFIF standard: https://www.w3.org/Graphics/JPEG/jfif3.pdf
 *  - Interesting note to not read 186 pages: https://www.opennet.ru/docs/formats/jpeg.txt
 *
 *  Markers: (https://www.disktuna.com/list-of-jpeg-markers/)
 *      FFD8: start of image file
 *      FFD9: end of image file
 *      All except FFD0 to FFD9 and FF01 are followed by length
 *
 *      APP0 is mandatory right after SOI (though this file starts with APP1...)
 *
 *  Colorspace:
 *      Use YCbCr. RGB calculated by linear conversion from YCbCr shall not be gamma corrected
 *      Only 1 component => Y
 *
 *
 *  Encoding:
 *      - RGB 8 bit each -> YCbCr all in the -128, 127 range
 *      - Sampling: luminance (Y) keep the high res sampling while CbCr are "downsampled" by a horizontal and vertical sampling coefficient (usually 2x2)
 *      - Image split into 8x8 blocks of pixels (each pixel with the 3 YCbCr components) (make it bigger if needed by repeating the last row/column)
 *          - processed left to right, top to bottom
 *      - For each 8x8 block, apply DCT separately on each of the 3 components
 *          - first block: Luminance
 *          - second block: Cb
 *          - third block: Cr
 *      - zig-zag reordering of the 64 DCT coeff
 *      - quantization: Each value from DCT are divided by a value found in the quantization table (vector of 64 values) and rounded to nearest int
 *
 *  ============
 *  JPEG EXIF
 *  ============
 *  https://en.wikipedia.org/wiki/Exif
 *  Basically compatible with JFIF except that it uses APP1 (FFE1) instead of APP0 (FFE0) with EXIF string instead of JFIF
 *
 */

// https://www.disktuna.com/list-of-jpeg-markers/
enum Markers {
    StartOfFrame0 = 0xFFC0,             // Baseline DCT
    StartOfFrame1 = 0xFFC1,             // Extended Sequential DCT
    StartOfFrame2 = 0xFFC2,             // Progressive DCT
    StartOfFrame3 = 0xFFC3,             // Lossless (sequential)

    DefineHuffmanTable = 0xFFC4,

    StartOfFrame5 = 0xFFC5,             // Differential sequential DCT
    StartOfFrame6 = 0xFFC6,             // Differential progressive DCT
    StartOfFrame7 = 0xFFC7,             // Differential lossless (sequential)

    JpegExtensions = 0xFFC8,

    StartOfFrame9 = 0xFFC9,             // Extended sequential DCT, Arithmetic coding
    StartOfFrame10 = 0xFFCA,            // Progressive DCT, Arithmetic coding
    StartOfFrame11 = 0xFFCB,            // Lossless (sequential), Arithmetic coding

    DefineArithmeticCoding = 0xFFCC,

    StartOfFrame13 = 0xFFCD,            // Differential sequential DCT, Arithmetic coding
    StartOfFrame14 = 0xFFCE,            // Differential progressive DCT, Arithmetic coding
    StartOfFrame15 = 0xFFCF,            // Differential lossless (sequential), Arithmetic coding

    Reset0 = 0xFFD0,                    // Reset marker
    Reset1 = 0xFFD1,
    Reset2 = 0xFFD2,
    Reset3 = 0xFFD3,
    Reset4 = 0xFFD4,
    Reset5 = 0xFFD5,
    Reset6 = 0xFFD6,
    Reset7 = 0xFFD7,

    StartOfImage            = 0xFFD8,
    EndOfImage              = 0xFFD9,
    StartOfScan             = 0xFFDA,
    DefineQuantizationTable = 0xFFDB,

    DefineNumberOfLines     = 0xFFDC,
    DefineRestartInterval   = 0xFFDD,
    DefineHierarchicalProgression = 0xFFDE,
    ExpandReferenceComponent = 0xFFDF,

    ApplicationSegment0 = 0xFFE0,       // JFIF jpeg image and AVI1 motion jpeg
    ApplicationSegment1 = 0xFFE1,       // EXIF metadata, TIFF ifd format, jpeg thumbnail, adobe xmp
    ApplicationSegment2 = 0xFFE2,       // ICC color profile, flash pix
    ApplicationSegment3 = 0xFFE3,       // Stereoscopic jpeg images
    ApplicationSegment4 = 0xFFE4,
    ApplicationSegment5 = 0xFFE5,
    ApplicationSegment6 = 0xFFE6,       // NITF lossless profile
    ApplicationSegment7 = 0xFFE7,
    ApplicationSegment8 = 0xFFE8,
    ApplicationSegment9 = 0xFFE9,
    ApplicationSegment10 = 0xFFEA,      // ActiveObject (multimedia messages/captions)
    ApplicationSegment11 = 0xFFEB,      // HELIOS JPEG Resources
    ApplicationSegment12 = 0xFFEC,      // Picture Info, Photoshop Save for Web: Ducky
    ApplicationSegment13 = 0xFFED,      // Photoshop Save As: IRB, 8BIM, IPTC
    ApplicationSegment14 = 0xFFEE,
    ApplicationSegment15 = 0xFFEF,

    JpegExtension0 = 0xFFF0,
    JpegExtension1 = 0xFFF1,
    JpegExtension2 = 0xFFF2,
    JpegExtension3 = 0xFFF3,
    JpegExtension4 = 0xFFF4,
    JpegExtension5 = 0xFFF5,
    JpegExtension6 = 0xFFF6,
    JpegExtension7 = 0xFFF7,            // Lossless jpeg
    JpegExtension8 = 0xFFF8,            // lossless jpeg extension parameters
    JpegExtension9 = 0xFFF9,
    JpegExtension10 = 0xFFFA,
    JpegExtension11 = 0xFFFB,
    JpegExtension12 = 0xFFFC,
    JpegExtension13 = 0xFFFD,

    Comment = 0xFFFE,
};

u32x3 rgb_to_ycbcr(u32x3 rgb) {
    /*
        Converts rgb 8-bit to YCbCr [0,255]
        RGB 8-bit each
        References:
        https://www.w3.org/Graphics/JPEG/jfif3.pdf page 4
    */
    u32x3 out = {0};
    out.Y  =    0.299f  * rgb.r + 0.587f  * rgb.g + 0.114f  * rgb.b;
    out.Cb =   -0.1687f * rgb.r - 0.3313f * rgb.g + 0.5f    * rgb.b + 128;
    out.Cr =    0.5f    * rgb.r - 0.4187f * rgb.g - 0.0813f * rgb.b + 128;
    return out;
}

u32x3* rgb_to_ycbcr(u32x3* rgb) {
    /*
        Converts rgb 8-bit to YCbCr [0,255]
        RGB 8-bit each
        References:
        https://www.w3.org/Graphics/JPEG/jfif3.pdf page 4
    */
    u32x3 in = *rgb;
    u32x3* out = rgb;
    out->Y  =    0.299f  * in.r + 0.587f  * in.g + 0.114f  * in.b;
    out->Cb =   -0.1687f * in.r - 0.3313f * in.g + 0.5f    * in.b + 128;
    out->Cr =    0.5f    * in.r - 0.4187f * in.g - 0.0813f * in.b + 128;
    return out;
}

u32Bytes argb_to_ycbcr(u32Bytes* rgb) {
    /*
        Converts rgb 8-bit to YCbCr [0,255]
        RGB 8-bit each
        References:
        https://www.w3.org/Graphics/JPEG/jfif3.pdf page 4
    */
    u32Bytes out = {0};
    out.Y  =    0.299f  * rgb->r + 0.587f  * rgb->g + 0.114f  * rgb->b;
    out.Cb =   -0.1687f * rgb->r - 0.3313f * rgb->g + 0.5f    * rgb->b + 128;
    out.Cr =    0.5f    * rgb->r - 0.4187f * rgb->g - 0.0813f * rgb->b + 128;
    return out;
}

s32x3 rgb_to_ycbcr_shifted(u32x3 rgb) {
    /*
        Converts rgb 8-bit to YCbCr [-128,127]
        RGB 8-bit each
        References:
        https://www.w3.org/Graphics/JPEG/jfif3.pdf page 4
    */
    s32x3 out = {0};
    out.Y  =   ( 0.299f  * rgb.r + 0.587f  * rgb.g + 0.114f  * rgb.b) - 128;
    out.Cb =   (-0.1687f * rgb.r - 0.3313f * rgb.g + 0.5f    * rgb.b);
    out.Cr =   ( 0.5f    * rgb.r - 0.4187f * rgb.g - 0.0813f * rgb.b);
    return out;
}

u32x3 ycbcr_to_rgb(u32x3 ycbcr) {
    u32& Y = ycbcr.Y;
    u32& Cb = ycbcr.Cb;
    u32& Cr = ycbcr.Cr;
    u32x3 out = {0};
    out.r = Y                       + 1.402f   * (Cr-128);
    out.g = Y - 0.34414f * (Cb-128) - 0.71414f * (Cr-128);
    out.b = Y + 1.772f   * (Cb-128);
    return out;
}

void DCT(void) {
    /*
     * Reference: https://www.w3.org/Graphics/JPEG/itu-t81.pdf page 31
     * There seems to exists and algo called AAN to compute it faster (with fewer multiplies / adds) AAN (Arai, Agui Nakajima in 1988)
     *  Y. Arai, T. Agui, and M. Nakajima, "A fast DCT-SQ scheme for images," Transactions of the IEICE, vol. 71, no. 11, pp. 1095-1097, 1988.
     *  Implementation at www.ijg.org
     *  Figure 4-8 in Pennebaker & Mitchel book: JPEG Still Image Data Compression Standard
     */
    // S[v,u] = 0.25f * C[u] * C[v] * (sum over all pixels in 8x8) (s[y,x] * cos((2*x+1)*u*pi / 16)) * cos((2*y+1) * v*pi / 16)
    // if u, v = 0 => Cu, Cv = 1 / sqrt(2)
    // else Cu, Cv = 1
}

void zigzag(void) {
    /*
      Order:
      0, 1, 5, 6,14,15,27,28,

      2, 4, 7,13,16,26,29,42,

      3, 8,12,17,25,30,41,43,

      9,11,18,24,31,40,44,53,

     10,19,23,32,39,45,52,54,

     20,22,33,38,46,51,55,60,

     21,34,37,47,50,56,59,61,

     35,36,48,49,57,58,62,63
     */
    // Element 0 is DC term
    // 63 following elements are AC
}

void quantization(void) {
    /*
     * for (i = 0 ; i<=63; i++ )
     *      vector[i] = (int) (vector[i] / quantization_table[i] + 0.5)
     */
}

void IDCT(void) {
    /*
     * Reference: https://www.w3.org/Graphics/JPEG/itu-t81.pdf page 31
     */
    // s[y, x] = 0.25f * (sum over all 8x8 pixels in freq. domain) * (C[u]*C[v] * S[v, u]) * cos((2*x+1)*u*pi/16) * cos((2*y+1)*v*pi / 16)
    // if u, v = 0 => Cu, Cv = 1 / sqrt(2)
    // else Cu, Cv = 1
}

void read_header();
void write_header();

u16 swap_endianness(u16 v) {
    u16 out = v<<8 | v>>8;
    return out;
}

void parse_StartOfFrame(u8** data) {
    u8* ptr = *data;
    // length in bytes for this chunk including the 2 bytes for the length variable itself
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    u8 hdr = *ptr;
    ptr += 1;
    if (hdr != 8) {
        printf("Bytes per color != 8 is not supported, file using: %d\n", hdr);
    }

    u32 height = swap_endianness(*(u16*)ptr);
    ptr += 2;

    u32 width = swap_endianness(*(u16*)ptr);
    ptr += 2;

    u32 components = *ptr;
    ptr += 1;

    printf("DCT length: %d, width: %d, height: %d, components: %d\n", length, width, height, components);

    // TODO(alex): continue here! (if needed, need to check what to do...)

    // Set the data pointer to the correct place
    *data += length;
}

void parse_ApplicationSegment0(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    *data += length;
}

void parse_ApplicationSegment1(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    *data += length;
}

void parse_ApplicationSegment13(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    *data += length;
}

void parse_ApplicationSegment14(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    *data += length;
}

void parse_DefineRestartInterval(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    *data += length;
}

void parse_DefineHuffmanTable(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    *data += length;
}

void parse_StartOfScan(u8** data) {
    u8* ptr = *data;
    // This is the length of the metadata until before the actual scan data
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;
    printf("Start of scan length: %d\n", length);

    ptr += length;
    u16 bytes = swap_endianness(*(u16*)ptr);
    while (bytes != EndOfImage) {
        ptr += 1;
        bytes = swap_endianness(*(u16*)ptr);
    }
    *data = ptr;
}

void parse_DefineQuantizationTable(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    *data += length;
}

Image decode_jpeg(string8 data) {
    // FFD8     StartOfImage
    // FFE1     App1
    //  FFD9    EndOfImage
    // FFED     App13
    // FFE1     App1
    u8* end_of_data = (u8*)(data.buffer + data.size);

    Image out = {0};
    u8* ptr = (u8*)data.buffer;
    u16 marker = swap_endianness(*(u16*)ptr);
    u16 length;
    u8 i=0;

    // Get start marker
    if (marker != StartOfImage) {
        printf("First marker is not StartOfImage!\n");
        return out;
    }

    while ( (ptr < end_of_data) && (marker!=EndOfImage) ) {
        // Increase pointer to after the marker
        ptr += 2;
        // TODO(alex): ptr automatically moved to the end of the data by each functions
        bool stop = false;
        switch (marker) {
            case StartOfImage: {
                printf("Start Of Image\n");
            } break;
            case ApplicationSegment0: {
                printf("File uses JFIF standard\n");
                parse_ApplicationSegment0(&ptr);
            } break;
            case ApplicationSegment1: {
                printf("File uses EXIF standard\n");
                parse_ApplicationSegment1(&ptr);
            } break;
            case ApplicationSegment13: {
                printf("Application Segment 13\n");
                parse_ApplicationSegment13(&ptr);
            } break;
            case ApplicationSegment14: {
                printf("Application Segment 14\n");
                parse_ApplicationSegment14(&ptr);
            } break;
            case DefineQuantizationTable: {
                printf("Define Quantization Table\n");
                parse_DefineQuantizationTable(&ptr);
            } break;
            case StartOfFrame0: {
                printf("Start Of Frame 0: Baseline DCT\n");
                parse_StartOfFrame(&ptr);
            } break;
            case DefineRestartInterval: {
                printf("Define Restart Interval\n");
                parse_DefineRestartInterval(&ptr);
            } break;
            case DefineHuffmanTable: {
                printf("Define Huffman Table\n");
                parse_DefineHuffmanTable(&ptr);
            } break;
            case StartOfScan: {
                printf("Start Of Scan\n");
                parse_StartOfScan(&ptr);
            } break;
            default: {
                printf("JPEG MARKER NOT SUPPORTED :: 0x%X\n", marker);
                stop = true;
            } break;
        }
        if (stop) break;
        marker = swap_endianness(*(u16*)ptr);
    }
    return out;
}

void write_jpeg(u32* rgb_data, u64 width, u64 height, string8 filepath) {
    LocalArena* local_arena = local_arena_alloc_create();

    u32Bytes* yc_data = (u32Bytes*)arena_alloc_push(local_arena->arena, width*height*sizeof(u32));
    u32Bytes* rgb_pixel = (u32Bytes*)rgb_data;

    const u8 block_size = 8;

    for (u64 y=0; y<height; y+=block_size) {
        for (u64 x=0; x<width; x+=block_size) {
            u32 block[block_size*block_size];

            // Convert rgb to YCbCr
            for (u8 j=0; j<block_size; j++) {
                for (u8 i=0; i<block_size; i++) {
                    if (i+x>width) {
                    }
                    if (j+y>height) {
                    }

                    rgb_pixel = (u32Bytes*)(rgb_data + (y*width+x + i+j*width));
                    u32Bytes tmp = argb_to_ycbcr(rgb_pixel);
                    block[j*block_size + i] = *(u32*)(&tmp);
                }
            }

            // DCT


        }
    }


    local_arena_alloc_reset(local_arena);
}

void write_image_file(u32* data, u64 width, u64 height, string8 filename) {
    string8 extension = string_get_file_extension(filename);

    if (extension == ".jpg" || extension == ".jpeg") {
        write_jpeg(data, width, height, filename);
    } else if (extension == ".png") {
        // write_png();
    } else {
        printf("Export to file not supported: extension = ");
        string_print(extension);
    }
}

Image create_missing_image(Arena* arena) {
    Image out = {0};
    out.width  = 128;
    out.height = 128;
    out.data = (u32*)arena_alloc_push(arena, out.width*out.height*sizeof(u32));

    const u32 pattern_size = 64;
    for (u32 j=0; j<out.height; j++) {
        for (u32 i=0; i<out.width; i++) {
            u32 linear_index = j*out.width + i;
            u32 color = 0;
            if (((u32)j/pattern_size + (u32)i/pattern_size) % 2 == 1) {
                color = 0xFF00FF;
            }
            out.data[linear_index] = color;
        }
    }
    return out;
}


Image read_image_file(Arena* arena, string8 filename) {
    LocalArena* local_arena = local_arena_alloc_create();

    printf("Reading file: ");
    string_print(filename);
    printf("\n");
    Image out = {0};
    string8 data = read_file(local_arena->arena, filename);
    string8 extension = string_get_file_extension(filename);

    if (extension == ".jpg" || extension == ".jpeg") {
        out = decode_jpeg(data);
    } else if (extension == ".png"){
        // out = decode_png(data);
    } else {
        printf("File format not supported: extension = ");
        string_print(extension);
    }

    if (!out.data) {
        out = create_missing_image(arena);
    }

    local_arena_alloc_reset(local_arena);
    return out;
}

#endif // LIB_IMAGES_IMPLEMENTATION
