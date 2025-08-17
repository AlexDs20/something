#ifndef _LIBIMAGES_H
#define _LIBIMAGES_H

#include "libstring.h"
#include "utils/types.h"
#include "platform/io.h"
// #include "libs/libmath.h"
// #include <math.h>

typedef struct ImageInfo ImageInfo;
struct ImageInfo {
    u32 width;
    u32 height;
    u8 channels;
    u8 precision;
};

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

// TODO(alex): Have real errors
typedef enum {
    JPEG_SUCCESS = 0,
    JPEG_FAIL,
} JpegError;

typedef struct JpegParsingResult JpegParsingResult;
struct JpegParsingResult {
    JpegError status;
    // TODO(alex): Make this string8?
    const char* error_message;
};

// TODO(alex):
// ImageInfo read_image_info(string8 filename);

// TODO(alex): Maybe request a pointer where there is enough space instead?
Image read_image_file(Arena* arena, string8 filename);

JpegParsingResult decode_jpeg(Arena* persist_arena, string8 data, Image* out);

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

#include "libs/IDCT_Weights.txt"

// itu-t81.pdf page 32
enum Markers {
    Temporary = 0xFF01,
    Reserved0 = 0xFF02,                 // 0xFF02 to 0xFFBF is reserved

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

    Restart0 = 0xFFD0,                    // Restart interval termination
    Restart1 = 0xFFD1,
    Restart2 = 0xFFD2,
    Restart3 = 0xFFD3,
    Restart4 = 0xFFD4,
    Restart5 = 0xFFD5,
    Restart6 = 0xFFD6,
    Restart7 = 0xFFD7,

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

void print_mk(u16 marker) {
    printf("Marker: 0x%X\n", marker);
}


typedef struct HuffmanNode HuffmanNode;
struct HuffmanNode {
    u8 value;
    u8 is_leaf;
    HuffmanNode* left;
    HuffmanNode* right;
};

typedef struct HuffmanTableSpec HuffmanTableSpec;
struct HuffmanTableSpec {
    u8 n_tables;

    u8 id[4];   // map index to id

    //                 Index
    HuffmanNode* DCroot[4];
    HuffmanNode* ACroot[4];
};

void draw_huffman_tree(HuffmanNode* node, int depth = 0, bool is_left = true) {
    if (!node) return;

    // Indentation
    for (int i = 0; i < depth; i++) {
        printf("    ");
    }

    // Print current node
    if (depth > 0) {
        printf("%s── ", is_left ? "L" : "R");
    }

    if (node->is_leaf) {
        printf("[Leaf: %u]\n", node->value);
    } else {
        printf("*\n");
    }

    // Recurse left and right
    draw_huffman_tree(node->left, depth + 1, true);
    draw_huffman_tree(node->right, depth + 1, false);
}

void print_ht(HuffmanTableSpec& ht, bool include_tree=false) {
    printf("Huffman table spec: \n");
    printf("------------------  \n");

    printf("Number tables: %d\n", ht.n_tables);
    for (u8 i=0; i<ht.n_tables; i++) {
        printf("Table %d:\n", i);
        printf("    id: %d\n", ht.id[i]);

        if (include_tree) {
            printf("    DCTree: \n    ");
            draw_huffman_tree(ht.DCroot[i]);
            printf("    ACTree: \n    ");
            draw_huffman_tree(ht.ACroot[i]);
        }
    }
}

typedef struct QuantizationTables QuantizationTables;
struct QuantizationTables {
    u8 n;
    u8 precision[4];                   // Precision of the Qk: 0 => 8bit, 1 => 16 bit
    u8 id[4];                          // Specifies where the quantization table to be used in the decoder
    u8 Q[4][64];                       // element in the zigzag ordering of the dct coefficients
};

void print_qt(QuantizationTables& qt) {
    u8 zigzag[8][8] = {
        { 0,  1,  5,  6, 14, 15, 27, 28},
        { 2,  4,  7, 13, 16, 26, 29, 42},
        { 3,  8, 12, 17, 25, 30, 41, 43},
        { 9, 11, 18, 24, 31, 40, 44, 53},
        {10, 19, 23, 32, 39, 45, 52, 54},
        {20, 22, 33, 38, 46, 51, 55, 60},
        {21, 34, 37, 47, 50, 56, 59, 61},
        {35, 36, 48, 49, 57, 58, 62, 63}
    };

    printf("%d Quantization Tables: \n", qt.n);
    printf("-------------------  \n");
    for (u8 i=0; i<qt.n; i++) {
        printf("Precision: %d\n", qt.precision[i]);
        printf("Identifier: %d\n", qt.id[i]);
        printf("Q: \n");
        for (u8 k=0;k<8; k++) {
            printf("    ");
            for (u8 j=0;j<8;j++) {
                printf(" %d ", qt.Q[i][zigzag[j][k]]);
            }
            printf("\n");
        }
        printf("\n");
    }
}

void print_zz(s16* d) {
    u8 zigzag[8][8] = {
        { 0,  1,  5,  6, 14, 15, 27, 28},
        { 2,  4,  7, 13, 16, 26, 29, 42},
        { 3,  8, 12, 17, 25, 30, 41, 43},
        { 9, 11, 18, 24, 31, 40, 44, 53},
        {10, 19, 23, 32, 39, 45, 52, 54},
        {20, 22, 33, 38, 46, 51, 55, 60},
        {21, 34, 37, 47, 50, 56, 59, 61},
        {35, 36, 48, 49, 57, 58, 62, 63}
    };

    for (u8 k=0;k<8; k++) {
        for (u8 j=0;j<8;j++) {
            printf("%d ", d[zigzag[j][k]]);
        }
        printf("\n");
    }
    printf("\n");
}

typedef struct ComponentInfo ComponentInfo;
struct ComponentInfo {
    HuffmanNode* DCHuffmanTable;
    HuffmanNode* ACHuffmanTable;
    u8* buffer;     // decoded data
    u8* QT;         // TODO
    u32 xi;
    u32 yi;
    u8 id;
    u8 Hi;          // Horizontal sapmling factor
    u8 Vi;          // Vertical sampling factor
};

void print_ci(ComponentInfo* ci) {
    printf("Component Info:\n");
    printf("==============\n");
    printf("  id: %d\n", ci->id);
    printf("  xi: %d\n", ci->xi);
    printf("  yi: %d\n", ci->yi);
    printf("  Hi: %d\n", ci->Hi);
    printf("  Vi: %d\n", ci->Vi);
    printf("  QT: %p\n", ci->QT);
    printf("  DC: %p\n", ci->DCHuffmanTable);
    printf("  AC: %p\n", ci->ACHuffmanTable);
}

typedef struct FrameHeader FrameHeader;
struct FrameHeader {
    union {
        u16 src_width;
        u16 X;
    };
    union {
        u16 src_height;
        u16 Y;
    };
    union {
        u8 src_precision;
        u8 P;
    };
    union {
        u8  src_components;
        u8  N;
    };

    u8 Hmax;
    u8 Vmax;
    u32 n_mcu_width;
    u32 n_mcu_height;

    u8 QT_selector[4];

    ComponentInfo components[4];
};

void print_fh(FrameHeader& fh) {
    printf("Frame Header: \n");
    printf("------------  \n");
    printf("Source image: \n");
    printf("  Precision: %d bits\n", fh.P);
    printf("  Width: %d\n", fh.X);
    printf("  Height: %d\n", fh.Y);
    printf("  Components: %d\n", fh.src_components);

    for (u8 i=0;i<fh.src_components; i++) {
        printf("Component %d:\n", i);
        printf("  id: %d\n", fh.components[i].id);
        printf("  H sample: %d\n", fh.components[i].Hi);
        printf("  V sample: %d\n", fh.components[i].Vi);
        printf("  QT selector: %d\n", fh.QT_selector[i]);
    }
}

typedef struct ScanHeader ScanHeader;
struct ScanHeader {
    u8 n_components;

    u8 id_selector[4];
    u8 dc_selector[4];
    u8 ac_selector[4];

    u8 start_spectral;              // Not used for baseline
    u8 end_spectral;                // Not used
    u8 approx_high;                 // Not used
    u8 approx_low;                  // Not used

    // TODO(alex): For simplicity, additional fields
    // ComponentInfo components[4];
    ComponentInfo* components[4];    // Do pointer to component info that is in the frame header
};

void print_sh(ScanHeader& sh) {
    printf("Scan Header: \n");
    printf("-----------  \n");

    for (u8 i=0; i<sh.n_components; i++) {
        printf("Component %d: \n", i);
        printf("    id selector: %d\n", sh.id_selector[i]);
        printf("    dc selector: %d\n", sh.dc_selector[i]);
        printf("    ac selector: %d\n", sh.ac_selector[i]);
    }
    printf("Start spectral: %d\n", sh.start_spectral);
    printf("End spectral: %d\n", sh.end_spectral);
    printf("Approx high: %d\n", sh.approx_high);
    printf("Approx low: %d\n", sh.approx_low);
}

typedef struct jpeg_t jpeg_t;
struct jpeg_t {
    QuantizationTables qt;
    FrameHeader fh;
    ScanHeader sh;
    HuffmanTableSpec ht;

    ComponentInfo component_info[4];
    u8 scan_components_index[4];        // Indices in the ComponentInfo for the current scan and its components (in order)

    u16 restart_interval;
    s16 dc_pred[4];
    u32 current_mcu;

    u8* buffer;
    Markers encoding;
};

typedef struct BitStream BitStream;
struct BitStream {
    u8* data;
    u64 size;
    u64 byte_pos;
    u8 bit_pos;
};

void print_bs(BitStream* bs) {
    printf("BitStream: \n");
    printf("---------\n");
    printf("  data*: %p\n", bs->data);
    printf("  size: %p\n", bs->size);
    printf("  byte: %d\n", bs->byte_pos);
    printf("  bit: %d\n", bs->bit_pos);
}

u8* current_ptr(BitStream* bs) {
    return bs->data + bs->byte_pos;
}

inline
bool overflow(BitStream* bs) {
    return bs->byte_pos >= bs->size;
}

inline
void skip_nbytes(BitStream* bs, u64 n) {
    bs->byte_pos += n;
}

u8 read_byte(BitStream* bs) {
    return bs->data[bs->byte_pos++];
}
u16 read_2bytes(BitStream* bs){
    return bs->data[bs->byte_pos++]<<8 | bs->data[bs->byte_pos++];
}
// u8 read_bit(BitStream* bs);

bool next_bit(BitStream* bs, u8* bit_out) {
    bool error = false;

    if (overflow(bs)) {
        printf("Next bit went beyond the bit stream data\n");
        error = true;
        return error;
    }

    u8 byte = bs->data[bs->byte_pos];
    *bit_out = (byte >> (7-bs->bit_pos)) & 1;

    // Update
    bs->bit_pos++;
    if (bs->bit_pos==8) {
        bs->bit_pos = 0;
        bs->byte_pos++;

        // Skip 0xFF00
        if (byte == 0xFF && bs->data[bs->byte_pos] == 0x00) {
            bs->byte_pos++;
        }
    }
    return error;
}

u8 peek_byte(BitStream* bs) {
    return bs->data[bs->byte_pos];
}
u16 peek_2bytes(BitStream* bs) {
    return bs->data[bs->byte_pos]<<8 | bs->data[bs->byte_pos+1];
}
// u8 peek_bit(BitStream* bs);

u16 peek_2_bytes(u8* v) {
    return v[0]<<8 | v[1];
}

// TODO(alex): move to libmath.h
u32 ceilf32(f32 a) {
    u32 aint = (u32)a;

    // If the value is its own int
    if (a == (f32)aint) {
        return aint;
    }

    if (a>0) {
        return aint+1;
    }
    return aint;
}

JpegParsingResult parse_FrameHeader(BitStream* bs, jpeg_t* jpeg) {
    FrameHeader& fh = jpeg->fh;

    u16 length = read_2bytes(bs);
    length -= 2;

    fh.src_precision = read_byte(bs);
    length--;

    if (fh.src_precision != 8) {
        return (JpegParsingResult){JPEG_FAIL, "Bytes per color != 8 not supported for DCT baseline."};
    }

    fh.src_height = read_2bytes(bs);
    // if 0 => should be gotten from DNL
    length -= 2;

    fh.src_width = read_2bytes(bs);
    length -= 2;

    if (fh.src_width == 0) {
        return (JpegParsingResult){JPEG_FAIL, "Missing image with data."};
    }

    fh.src_components = read_byte(bs);
    length--;

    if (fh.src_components == 0) {
        return (JpegParsingResult){JPEG_FAIL, "Number of components = 0 not supported."};
    }

    u8 v;
    for (u8 i=0; i<fh.src_components; i++) {
        // If components id are 1 2 and 3 => usually YCbCr
        //  else if 0x52, 0x47, 0x42 => RGB (very rare)
        fh.components[i].id = read_byte(bs);
        length--;

        v = read_byte(bs);
        length--;
        fh.components[i].Hi = v >> 4;
        fh.components[i].Vi = v & 0x0F;

        if (fh.components[i].Hi > 4 || fh.components[i].Hi == 0) {
            return (JpegParsingResult){JPEG_FAIL, "Horizontal sampling factor is not between 1 and 4."};
        } else if (fh.components[i].Vi > 4 || fh.components[i].Vi == 0) {
            return (JpegParsingResult){JPEG_FAIL, "Vertical sampling factor is not between 1 and 4."};
        }

        fh.Hmax = fh.components[i].Hi > fh.Hmax ? fh.components[i].Hi : fh.Hmax;
        fh.Vmax = fh.components[i].Vi > fh.Vmax ? fh.components[i].Vi : fh.Vmax;

        fh.QT_selector[i] = read_byte(bs);
        length--;
        if (fh.QT_selector[i] > 3) {
            return (JpegParsingResult){JPEG_FAIL, "Quantization table selector must be between 0 and 3."};
        }
    }

    if (length != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Expected length of Start of Frame header and read data does not match."};
    }

    // Compute interesting size info related to mcu and components
    fh.n_mcu_width  = ceilf32((f32)fh.X / (8*(u16)fh.Hmax));
    fh.n_mcu_height = ceilf32((f32)fh.Y / (8*(u16)fh.Vmax));

    // TODO(alex): I should also allocate temporary memory for these to hold the actual component values after decoding
    for (u8 i=0; i<fh.src_components; i++) {
        fh.components[i].xi = fh.n_mcu_width  * (8*fh.components[i].Hi);
        fh.components[i].yi = fh.n_mcu_height * (8*fh.components[i].Vi);
        print_ci(&fh.components[i]);
    }

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_ScanHeader(BitStream* bs, jpeg_t* jpeg) {
    ScanHeader& sh = jpeg->sh;

    u16 length = read_2bytes(bs);
    length -= 2;

    // Header
    sh.n_components = read_byte(bs);
    length--;
    if (sh.n_components > 4 || sh.n_components == 0) {
        return (JpegParsingResult){JPEG_FAIL, "Number of components in a scan must be between 1 and 3."};
    }

    u8 v;
    for (u8 i=0; i<sh.n_components; i++) {
        sh.id_selector[i] = read_byte(bs);
        length--;

        v = read_byte(bs);
        length--;
        sh.dc_selector[i] = v >> 4;
        sh.ac_selector[i] = v & 0x0F;
        if (sh.dc_selector[i] > 1) {
            return (JpegParsingResult){JPEG_FAIL, "DC coding table must be 0 or 1 for Sequential Baseline DCT."};
        } else if (sh.ac_selector[i] > 1) {
            return (JpegParsingResult){JPEG_FAIL, "AC coding table must be 0 or 1 for Sequential Baseline DCT."};
        }

        // Setup the component informations so that it's complete with QT and AC/DC tables
        for (u8 j=0; j<jpeg->fh.N; j++) {
            if (sh.id_selector[i] == jpeg->fh.components[j].id) {
                sh.components[i] = &jpeg->fh.components[j];

                for (u8 k=0; k<jpeg->ht.n_tables; k++) {
                    if (sh.dc_selector[i] == jpeg->ht.id[k]) {
                        sh.components[i]->DCHuffmanTable = jpeg->ht.DCroot[k];
                    }
                    if (sh.ac_selector[i] == jpeg->ht.id[k]) {
                        sh.components[i]->ACHuffmanTable = jpeg->ht.ACroot[k];
                    }
                }
                break;
            }
        }
    }

    sh.start_spectral = read_byte(bs);
    length--;
    if (sh.start_spectral != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Baseline DCT must have start spectral = 0"};
    }

    sh.end_spectral = read_byte(bs);
    length--;
    if (sh.end_spectral != 63) {
        return (JpegParsingResult){JPEG_FAIL, "Baseline DCT must have end spectral = 0"};
    }

    v = read_byte(bs);
    length--;
    sh.approx_high = v >> 4;
    if (sh.approx_high != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Baseline DCT must have approximation high = 0"};
    }
    sh.approx_low  = v & 0x0F;
    if (sh.approx_low != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Baseline DCT must have approximation low = 0"};
    }

    if (length != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Expected length of Start of Scan header and read data do not match."};
    }

    return (JpegParsingResult){JPEG_SUCCESS, "Successfully read jpeg file!"};
}

JpegParsingResult parse_ApplicationSegmentN(BitStream* bs, jpeg_t* jpeg) {
    u16 length = peek_2bytes(bs);
    skip_nbytes(bs, length);
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_ApplicationSegment14(BitStream* bs, jpeg_t* jpeg) {
    // This is usually used by adobe encoders
    //
    // Adobed@
    // Adobed
    // Usually contains a color transform flag at offset 11
    //  value: 0=>RGB, 1=>YCbCr(default), 2=>YCCK

    u16 length = peek_2bytes(bs);
    skip_nbytes(bs, length);
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_Comment(BitStream* bs, jpeg_t* jpeg) {
    u16 length = peek_2bytes(bs);
    skip_nbytes(bs, length);
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_DefineRestartInterval(BitStream* bs, jpeg_t* jpeg) {
    u16 length = read_2bytes(bs);
    length -= 2;

    // Specifies the number of MCU in the restart interval
    jpeg->restart_interval = read_2bytes(bs);
    length -= 2;

    if (length != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Expected length of DefineRestartInterval and read data do not match."};
    }

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_DefineNumberOfLines(BitStream* bs, jpeg_t* jpeg) {
    u16 length = read_2bytes(bs);
    length -= 2;

    // Specifies the number of MCU in the restart interval
    jpeg->fh.src_height = read_2bytes(bs);
    length -= 2;

    if (jpeg->fh.src_height == 0) {
        return (JpegParsingResult){JPEG_FAIL, "Assigned number of lines to 0 in DefineNumberOfLines."};
    }

    if (length != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Expected length of DefineNumberOfLines and read data do not match."};
    }

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_DefineHuffmanTable(Arena* arena, BitStream* bs, jpeg_t* jpeg) {
    HuffmanTableSpec& ht = jpeg->ht;

    s32 length = (s32)read_2bytes(bs);
    length -= 2;

    u8 n_tables = 0;
    u8 v;
    while (length > 0) {
        v = read_byte(bs);
        length--;

        // Get the table type and id
        u8 table_class = v >> 4;                 // 0 DC or lossless  --- 1 AC
        u8 table_id = v & 0x0F;

        // Check if we already have the id (because same id for DC and AC)
        u8 idx = n_tables;
        for (u8 i=0; i<n_tables; i++) {
            if (table_id == ht.id[i]) {
                idx = i;
                break;
            }
        }
        // If we don't have the id, increase number of tables
        if (idx == n_tables) {
            n_tables++;
        }

        ht.id[idx] = table_id;

        // For each length of symbols save that length and count the total number of symbols
        u16 num_symbols = 0;
        u16 code_lengths[16];
        for (u8 i=0; i<16; i++) {
            v = read_byte(bs);
            length--;

            code_lengths[i] = v;
            num_symbols += v;
        }

        // Allocate space for all symbols
        u8* symbols = (u8*)arena_alloc_push_struct(arena, current_ptr(bs), num_symbols*sizeof(u8));
        skip_nbytes(bs, num_symbols);
        length -= num_symbols;

        //==============================
        // Convert table to tree for easier parsing of the entropy stream
        HuffmanNode* root = (HuffmanNode*)arena_alloc_push_zero_unaligned(arena, sizeof(HuffmanNode));
        if (table_class == 0) {
            ht.DCroot[idx] = root;
        } else {
            ht.ACroot[idx] = root;
        }

        HuffmanNode* node = 0;
        u16 code = 0;
        u8* symbol = symbols;

        for (u8 i=0; i<16; i++) {
            u8 l = i+1;

            // For each code of length i
            for (u16 j=0; j<code_lengths[i]; j++, code++, symbol++) {
                node = root;

                // Use binary coding to traverse (and create) tree and assign value to node
                for (s16 c=l-1; c>=0; --c) {
                    u8 bit = (code >> c) & 1;
                    if (bit) { // right
                        if (!node->right) {
                            node->right = (HuffmanNode*)arena_alloc_push_zero_unaligned(arena, sizeof(HuffmanNode));
                        }
                        node = node->right;
                    } else {    // left
                        if (!node->left) {
                            node->left = (HuffmanNode*)arena_alloc_push_zero_unaligned(arena, sizeof(HuffmanNode));
                        }
                        node = node->left;
                    }
                }
                node->is_leaf = 1;
                node->value = *symbol;
            }

            code <<= 1;
        }
    }

    ht.n_tables = n_tables;

    if (length != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Expected length of DHT and read data does not match."};
    }

    return (JpegParsingResult){JPEG_SUCCESS, "DHT"};
}

JpegParsingResult parse_DefineQuantizationTable(BitStream* bs, jpeg_t* jpeg) {
    QuantizationTables& qt = jpeg->qt;

    u16 length = read_2bytes(bs);
    length -= 2;

    u8 table_idx = 0;
    qt.n = 0;
    while (length > 0) {
        qt.n++;

        u8 v = read_byte(bs);
        length--;
        qt.precision[table_idx] = v >> 4;
        qt.id[table_idx]  = v & 0x0F;

        if (qt.precision[table_idx] == 1) {
            return (JpegParsingResult){JPEG_FAIL, "Quantization table precision == 16 bits not supported!"};
        } else if (qt.precision[table_idx] > 1) {
            return (JpegParsingResult){JPEG_FAIL, "Quantization table precision value not supported!"};
        }

        if (qt.id[table_idx]>3) {
            return (JpegParsingResult){JPEG_FAIL, "Quantization table identifier should be between 0 and 3!"};
        }

        // Set the qt table to the matching component
        for (u8 j=0; j<jpeg->fh.N; j++) {
            if (qt.id[table_idx] == jpeg->fh.QT_selector[j]) {
                jpeg->fh.components[j].QT = &qt.Q[table_idx][0];
                break;
            }
        }

        for (u8 i=0; i<64; i++) {
            qt.Q[table_idx][i] = read_byte(bs);
            length--;
        }
        table_idx++;
    }

    if (length != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Expected length of DQT and read data does not match."};
    }

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

bool is_start_of_frame(u16 marker) {
    return marker==StartOfFrame0 ||
           marker==StartOfFrame1 ||
           marker==StartOfFrame2 ||
           marker==StartOfFrame3 ||
           marker==StartOfFrame5 ||
           marker==StartOfFrame6 ||
           marker==StartOfFrame7;
}

bool is_restart_marker(u16 marker, s8* idx) {
    bool out = true;
    switch (marker) {
        case Restart0: {
            *idx = 0;
        } break;
        case Restart1: {
            *idx = 1;
        } break;
        case Restart2: {
            *idx = 2;
        } break;
        case Restart3: {
            *idx = 3;
        } break;
        case Restart4: {
            *idx = 4;
        } break;
        case Restart5: {
            *idx = 5;
        } break;
        case Restart6: {
            *idx = 6;
        } break;
        case Restart7: {
            *idx = 7;
        } break;
        default: {
            out = false;
        } break;
    }
    return out;
}

bool is_interpret_marker(u16 marker) {
    bool out = false;
    switch (marker) {
        case DefineQuantizationTable:
        case DefineHuffmanTable:
        case DefineArithmeticCoding:
        case DefineRestartInterval:
        case Comment:
        case DefineNumberOfLines:
        case ApplicationSegment0:
        case ApplicationSegment1:
        case ApplicationSegment2:
        case ApplicationSegment3:
        case ApplicationSegment4:
        case ApplicationSegment5:
        case ApplicationSegment6:
        case ApplicationSegment7:
        case ApplicationSegment8:
        case ApplicationSegment9:
        case ApplicationSegment10:
        case ApplicationSegment11:
        case ApplicationSegment12:
        case ApplicationSegment13:
        case ApplicationSegment14:
        case ApplicationSegment15: {
            out = true;
        } break;
    }
    return out;
}

JpegParsingResult interpret_marker(Arena* arena, BitStream* bs, jpeg_t* jpeg) {
    u16 marker = read_2bytes(bs);
    JpegParsingResult result;

    switch (marker) {
        case ApplicationSegment0:
        case ApplicationSegment1:
        case ApplicationSegment13: {
            result = parse_ApplicationSegmentN(bs, jpeg);
        } break;
        case ApplicationSegment14: {
            printf("ApplicationSegment14\n");
            result = parse_ApplicationSegment14(bs, jpeg);
        } break;
        case DefineQuantizationTable: {
            printf("DefineQuantizationTable\n");
            result = parse_DefineQuantizationTable(bs, jpeg);
        } break;
        case DefineRestartInterval: {
            printf("DefineRestartInterval\n");
            result = parse_DefineRestartInterval(bs, jpeg);
        } break;
        case DefineHuffmanTable: {
            printf("DefineHuffmanTable\n");
            result = parse_DefineHuffmanTable(arena, bs, jpeg);
        } break;
        case Comment: {
            printf("Comment\n");
            result = parse_Comment(bs, jpeg);
        } break;
        case DefineNumberOfLines: {
            printf("DefineNumberOfLines\n");
            result = parse_DefineNumberOfLines(bs, jpeg);
        } break;
    }

    return result;
}

JpegParsingResult decode_one_Huffman_code(BitStream* bs, HuffmanNode* root, u8* result) {
    HuffmanNode* node = root;

    // Huffman decode category which is the number of bits to decode after
    u8 bit = 0;
    while (!node->is_leaf) {
        next_bit(bs, &bit);
        if (bit) {
            node = node->right;
            if (node == nullptr){
                return (JpegParsingResult){JPEG_FAIL, "Failed decoding node going right."};
            }
        } else {
            node = node->left;
            if (node == nullptr){
                return (JpegParsingResult){JPEG_FAIL, "Failed decoding node going left."};
            }
        }
    }

    *result = node->value;

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

s16 parse_n_bits(BitStream* bs, u8 n) {
    s16 out;
    u8 bit;
    u16 tmp_value = 0;
    for (u8 i=0; i<n; i++) {
        next_bit(bs, &bit);
        tmp_value = (tmp_value<<1) | bit;
    }

    // Convert to signed value
    if ( tmp_value < (1 << (n-1)) ) {        // check if first bit set is 0 => negative
        out = (s16)(tmp_value - (1 << n) + 1);
    } else {
        out = (s16)tmp_value;
    }
    return out;
}

JpegParsingResult parse_DCDataUnit(BitStream* bs, HuffmanNode* root, s16* out) {
    u8 category;
    decode_one_Huffman_code(bs, root, &category);

    // If category 0 => value = 0 => early exit
    if (category == 0) {
        *out = 0;
        return (JpegParsingResult){JPEG_SUCCESS, 0};
    }

    // Read that amount of bits
    *out = parse_n_bits(bs, category);

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_ACDataUnit(BitStream* bs, HuffmanNode* root, u8* run_length, s16* out) {
    u8 symbol;
    decode_one_Huffman_code(bs, root, &symbol);

    *run_length = symbol >> 4;
    u8 category = symbol & 0x0F;

    if (category == 0) {
        *out = 0;
        return (JpegParsingResult){JPEG_SUCCESS, 0};
    }

    *out = parse_n_bits(bs, category);

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

s16 clamp(s16 a, s16 low=0, s16 high=255){
    s16 t = a < low ? low : a;
    return t > high ? high : t;
}

JpegParsingResult parse_mcu(Arena* arena, BitStream* bs, jpeg_t* jpeg) {
    u8 n_components = jpeg->sh.n_components;

    const u8 DC = 0;
    const u8 AC = 1;
    const u32 PI = 3.1415926535;
    // To go from flat zigzag order to flat unzigzag order
    const u8 unzigzag[64] = {
        0,   1,  8, 16,  9,  2,  3, 10,
        17, 24, 32, 25, 18, 11,  4,  5,
        12, 19, 26, 33, 40, 48, 41, 34,
        27, 20, 13,  6,  7, 14, 21, 28,
        35, 42, 49, 56, 57, 50, 43, 36,
        29, 22, 15, 23, 30, 37, 44, 51,
        58, 59, 52, 45, 38, 31, 39, 46,
        53, 60, 61, 54, 47, 55, 62, 63
    };

    // This is valid for the whole scan!
    u8 component_idx[4] = {0};
    for (u8 j=0; j<n_components; j++) {
        u8 cs_j = jpeg->sh.id_selector[j];

        for (u8 i=0; i<jpeg->fh.src_components; i++) {
            u8 c_i = jpeg->fh.components[i].id;
            if (c_i == cs_j) {
                component_idx[j] = i;
                break;
            }
        }
    }


    s16 zz_mcu[4][64] = {0};
    s16 mcu[4][64] = {0};
    f32 idct[4][64] = {0};
    for (u8 i=0; i<n_components; i++) {
        // Get the correct component index in the frame header
        u8 idx = component_idx[i];

        u8 qt_idx = jpeg->fh.QT_selector[idx];
        u8* Q = jpeg->qt.Q[qt_idx];

        u8 dc_table_id = jpeg->sh.dc_selector[i];
        u8 ac_table_id = jpeg->sh.ac_selector[i];

        HuffmanNode* dc_ht_root = 0;
        HuffmanNode* ac_ht_root = 0;
        for (u8 j=0; j<jpeg->ht.n_tables; j++) {
            if (dc_table_id == jpeg->ht.id[j]) {
                dc_ht_root = jpeg->ht.DCroot[j];
            }
            if (ac_table_id == jpeg->ht.id[j]) {
                ac_ht_root = jpeg->ht.ACroot[j];
            }
        }

        for (u8 v=0; v<jpeg->fh.components[idx].Vi; v++) {
            for (u8 h=0; h<jpeg->fh.components[idx].Hi; h++) {
                s16 value;
                JpegParsingResult result = parse_DCDataUnit(bs, dc_ht_root, &value);
                if (result.status != JPEG_SUCCESS) { return result; }
                s16 dc = jpeg->dc_pred[idx] + value;
                jpeg->dc_pred[idx] = dc;
                zz_mcu[idx][0] = dc;

                u8 j = 1;
                while (j<64) {        // and not a marker that would indicate something
                    u8 preceding_zeros = 0;
                    JpegParsingResult result = parse_ACDataUnit(bs, ac_ht_root, &preceding_zeros, &value);
                    s16 ac = value;

                    if (ac == 0 && preceding_zeros == 0) {
                        while (j<64) {
                            zz_mcu[idx][j++] = 0;
                        }
                    } else {
                        for (u8 k=0; k<preceding_zeros; k++) {
                            zz_mcu[idx][j++] = 0;
                        }
                        zz_mcu[idx][j++] = ac;
                    }
                }
            }
        }

        // Dequantize
        for (u8 l=0; l<64; l++) {
            zz_mcu[idx][l] *= Q[l];
        }

        // Unzigzag
        for (u8 l=0; l<64; l++) {
            mcu[idx][unzigzag[l]] = zz_mcu[idx][l];
        }

        // IDCT
        for (u8 y=0; y<8; y++) {
            for (u8 x=0; x<8; x++) {
                u8 l = x+y*8;

                for (u8 v=0; v<8; v++) {
                    f32 Cv = v==0 ? 0.7071067811 : 1;
                    for (u8 u=0; u<8; u++) {
                        f32 Cu = u==0 ? 0.7071067811 : 1;

                        u8 l_vu = u + v*8;
                        idct[idx][l] += (f64)(Cu * Cv * mcu[idx][l_vu] * IDCT_Weights[x][u]*IDCT_Weights[y][v]);
                    }
                }

                f32 a = (f32)(0.25f*idct[idx][l] + 128);
                idct[idx][l] = clamp(a+0.5);
            }
        }
    }

    // Assume YCbCr
    // Convert to RGB
    // (0,0)
    u16 n_blocks_x = (u16)((jpeg->fh.src_width  + 7) / 8.0f);
    u16 n_blocks_y = (u16)((jpeg->fh.src_height + 7) / 8.0f);
    u16 start_x = jpeg->current_mcu % n_blocks_x;
    u16 start_y = (u16)(jpeg->current_mcu / n_blocks_x);
    if (n_components==3) {
        // printf("n blocks: (%d, %d)  start: (%d, %d)\n", n_blocks_y, n_blocks_x, start_y, start_x);
        for (u8 y=0; y<8; y++) {
            for (u8 x=0; x<8; x++) {
                u8 l = x + y * 8;
                f32 r = idct[0][l]                                + 1.402   * (idct[2][l] - 128);
                f32 g = idct[0][l] - 0.34414 * (idct[1][l] - 128) - 0.71414 * (idct[2][l] - 128);
                f32 b = idct[0][l] + 1.772   * (idct[1][l] - 128);

                // TODO check not going over edge
                u32 NX = (start_x*8 + x);
                u32 NY = (start_y*8 + y);
                u32 linear = NX + NY*jpeg->fh.src_width;
                jpeg->buffer[linear*4 + 0] = (u8)clamp(b+0.5);
                jpeg->buffer[linear*4 + 1] = (u8)clamp(g+0.5);
                jpeg->buffer[linear*4 + 2] = (u8)clamp(r+0.5);
            }
        }
    } else {
        for (u8 y=0; y<8; y++) {
            for (u8 x=0; x<8; x++) {
                u8 l = x + y * 8;

                u32 NX = (start_x*8 + x);
                u32 NY = (start_y*8 + y);
                u32 linear = NX + NY*jpeg->fh.src_width;

                jpeg->buffer[linear] = (u8)clamp(idct[0][l] + 0.5f);
            }
        }
    }
    jpeg->current_mcu++;

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_EntropySegment(Arena* arena, BitStream* bs, jpeg_t* jpeg) {
    u16 marker;

    if (jpeg->restart_interval == 0) {
        return (JpegParsingResult){JPEG_FAIL, "No restart interval is currently unsupported."};
    }

    s8 tmp;
    u16 n = 0;

    // Reset all 4 u8 values to 0
    jpeg->dc_pred[0] = 0;
    jpeg->dc_pred[1] = 0;
    jpeg->dc_pred[2] = 0;
    jpeg->dc_pred[3] = 0;
    while (n++ < jpeg->restart_interval) {
        JpegParsingResult result = parse_mcu(arena, bs, jpeg);
        if (result.status != JPEG_SUCCESS) { return result; }
    }

    // Go through all the last bits until byte aligned
    u8 bit;
    while (bs->bit_pos != 0) {
        next_bit(bs, &bit);
    }

    marker = peek_2bytes(bs);
    if (!is_restart_marker(marker, &tmp) && !is_interpret_marker(marker) && marker != StartOfScan && marker != EndOfImage) {
        return (JpegParsingResult){JPEG_FAIL, "Invalid marker encountered."};
    }

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_Scan(Arena* persist_arena, Arena* local_arena, BitStream* bs, jpeg_t* jpeg) {
    u16 marker = peek_2bytes(bs);

    while (is_interpret_marker(marker) && !overflow(bs)) {
        JpegParsingResult result = interpret_marker(local_arena, bs, jpeg);
        if (result.status != JPEG_SUCCESS) { return result; }
        marker = peek_2bytes(bs);
    }

    if (marker != StartOfScan) {
        return (JpegParsingResult){JPEG_FAIL, "Did not find Start of Scan header marker!"};
    }

    skip_nbytes(bs, 2);
    JpegParsingResult result = parse_ScanHeader(bs, jpeg);
    if (result.status != JPEG_SUCCESS) { return result; }

    s8 restart_id = -1;
    while (true) {
        JpegParsingResult result = parse_EntropySegment(persist_arena, bs, jpeg);
        if (result.status != JPEG_SUCCESS) {
            // TODO(alex): Here could check and move forward until next restart marker?
            return result;
        }
        marker = peek_2bytes(bs);
        if (is_restart_marker(marker, &restart_id)) {
            skip_nbytes(bs, 2);
            continue;
        } else {
            break;
        }
    }

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult decode_frame(Arena* persist_arena, BitStream* bs, jpeg_t* jpeg) {
    LocalArena* local_arena = local_arena_alloc_create();

    // Parse [Tables/misc.] of the frame
    u16 marker = peek_2bytes(bs);
    while (is_interpret_marker(marker) && !overflow(bs)) {
        JpegParsingResult result = interpret_marker(local_arena->arena, bs, jpeg);

        if (result.status != JPEG_SUCCESS) { return result; }
        marker = peek_2bytes(bs);
    }

    if (overflow(bs)) {
        return (JpegParsingResult){JPEG_FAIL, "Trying to parse beyond the end of file!"};
    }

    if (!is_start_of_frame(marker)) {
        return (JpegParsingResult){JPEG_FAIL, "Did not find start of frame marker!"};
    }

    // Parse Frame header
    skip_nbytes(bs, 2);
    if (marker == StartOfFrame0) {
        jpeg->encoding = (Markers)marker;
        printf("Parse start of frame!\n");
        JpegParsingResult result = parse_FrameHeader(bs, jpeg);

        if (result.status != JPEG_SUCCESS) { return result; }
    } else {
        return (JpegParsingResult){JPEG_FAIL, "Only StartOfFrame0 marker implemented!"};
    }

    // TODO(alex): Allocate precisely the right amount and use u8* regardless
    // Maybe we should alloc in a local arena for an image of the size that is a multiple of the number of blocks one actually needs because of padding
    // jpeg->buffer = (u8*)arena_alloc_push(persist_arena, jpeg->fh.src_width*jpeg->fh.src_height*jpeg->fh.src_components);
    jpeg->buffer = (u8*)arena_alloc_push(persist_arena, jpeg->fh.src_width*jpeg->fh.src_height*4*sizeof(u8));

    marker = peek_2bytes(bs);
    while ((is_interpret_marker(marker) || (marker==StartOfScan)) && !overflow(bs)) {       // Either DNL between scans or Tables/misc. or Scan header
        JpegParsingResult result = parse_Scan(persist_arena, local_arena->arena, bs, jpeg);

        if (result.status != JPEG_SUCCESS) { return result; }
        marker = peek_2bytes(bs);
    }

    if (overflow(bs)) {
        return (JpegParsingResult){JPEG_FAIL, "Trying to parse beyond the end of file!"};
    }

    local_arena_alloc_reset(local_arena);
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult decode_jpeg(Arena* persist_arena, string8 data, Image* out) {
    // Note: Current goal is to support baseline DCT 8 bits
    BitStream bs = {
        .data = data.buffer,
        .size = data.size,
        .byte_pos = 0,
        .bit_pos = 0
    };
    u16 marker = read_2bytes(&bs);
    u16 end_marker = bs.data[bs.size-2]<<8 | bs.data[bs.size-1];

    // Get start marker
    if (marker != StartOfImage) {
        return (JpegParsingResult){JPEG_FAIL, "Missing Start Of Image marker"};
    } else if (end_marker != EndOfImage) {
        return (JpegParsingResult){JPEG_FAIL, "Missing End Of Image marker"};
    }

    // Decoder_setup
    jpeg_t jpeg = {0};

    JpegParsingResult result = decode_frame(persist_arena, &bs, &jpeg);

    if (result.status != JPEG_SUCCESS) { return result; }

    marker = peek_2bytes(&bs);
    if (marker != EndOfImage) {
        return (JpegParsingResult){JPEG_FAIL, "Did not find End Of Image marker after decoding scan(s)!"};
    }

    out->width       = jpeg.fh.src_width;
    out->height      = jpeg.fh.src_height;
    out->components  = jpeg.fh.src_components;
    out->precision   = jpeg.fh.src_precision;
    out->buffer      = (u32*)jpeg.buffer;

    return (JpegParsingResult){JPEG_SUCCESS, "Successfully read jpeg file!"};
}

void create_missing_image(Arena* arena, Image* out) {
    out->width  = 128;
    out->height = 128;
    out->data = (u32*)arena_alloc_push(arena, out->width*out->height*sizeof(u32));

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

Image read_image_file(Arena* persist_arena, string8 filename) {
    Image out = {0};
    LocalArena* local_arena = local_arena_alloc_create();
    u64 checkpoint = arena_alloc_checkpoint(persist_arena);

    printf("Reading file: ");
    string_print(filename);
    printf("\n");
    string8 data = read_file(local_arena->arena, filename);

    string8 extension = string_get_file_extension(filename);
    if (extension == ".jpg" || extension == ".jpeg") {
        JpegParsingResult result = decode_jpeg(persist_arena, data, &out);

        if (result.status != JPEG_SUCCESS) {
            arena_alloc_restore_zero(persist_arena, checkpoint);
            printf("Failed parsing jpeg file: ");
            string_print(filename);
            printf(" with error: %s\n", result.error_message);
        }
    } else if (extension == ".png"){
        // decode_png(persist_arena, data, &out);
    } else {
        printf("File format not supported: extension = ");
        string_print(extension);
    }

    if (!out.data) {
        create_missing_image(persist_arena, &out);
    }

    local_arena_alloc_reset(local_arena);
    return out;
}

#endif // LIB_IMAGES_IMPLEMENTATION
