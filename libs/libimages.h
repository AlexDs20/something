#ifndef _LIBIMAGES_H
#define _LIBIMAGES_H

#include "libstring.h"
#include "utils/types.h"
#include "platform/io.h"
// #include "libs/libmath.h"
#include <math.h>

Image read_image_file(Arena* arena, string8 filename);
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

// TODO(alex): Have real errors
typedef enum {
    JPEG_SUCCESS = 0,
    JPEG_INVALID_HEADER,
    JPEG_MISSING_MARKER,
    JPEG_FAIL,
} JpegError;

typedef struct {
    JpegError status;
    // TODO(alex): Make this string8?
    const char* error_message;
} JpegParsingResult;

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

struct FrameHeader {
    u8  src_precision;
    u16 src_height;
    u16 src_width;
    u8  src_components;

    u8 component_id[4];
    u8 H_sample[4];
    u8 V_sample[4];
    u8 QT_selector[4];
};

struct ScanHeader {
    u8 n_components;

    u8 component_selector[4];
    u8 dc_selector[4];
    u8 ac_selector[4];

    u8 start_spectral;              // Not used for baseline
    u8 end_spectral;                // Not used
    u8 approx_high;                 // Not used
    u8 approx_low;                  // Not used

    // TODO(alex): For simplicity, additional fields
    // u8 id_component_fh[4];
};

struct HuffmanNode{
    u8 value;
    u8 is_leaf;
    HuffmanNode* left;
    HuffmanNode* right;
};

struct HuffmanTableSpec {
    u8 n_tables;

    u8 table_class[8];
    u8 table_id[8];
    u8 code_lengths[8][16];
    u8* symbols[8];
    u16 num_symbols[8];

    //             AC/DC  ID
    HuffmanNode* root[2] [4];
};

struct QuantizationTables {
    u8 n;
    u8 precision[4];                   // Precision of the Qk: 0 => 8bit, 1 => 16 bit
    u8 id[4];                          // Specifies where the quantization table to be used in the decoder
    u8 Q[4][64];                       // element in the zigzag ordering of the dct coefficients
};

typedef struct {
    u8* data;
    u8* end;
    u64 byte_pos;
    u8 bit_pos;
} BitStream;

void print_fh(FrameHeader& fh) {
    printf("Frame Header: \n");
    printf("------------  \n");
    printf("Source image: \n");
    printf("  Precision: %d bits\n", fh.src_precision);
    printf("  Height: %d\n", fh.src_height);
    printf("  Width: %d\n", fh.src_width);
    printf("  Components: %d\n", fh.src_components);

    for (u8 i=0;i<fh.src_components; i++) {
        printf("Component %d:\n", i);
        printf("  id: %d\n", fh.component_id[i]);
        printf("  H sample: %d\n", fh.H_sample[i]);
        printf("  V sample: %d\n", fh.V_sample[i]);
        printf("  QT selector: %d\n", fh.QT_selector[i]);
    }
}

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
        printf("    class: %d\n", ht.table_class[i]);
        printf("    id: %d\n", ht.table_id[i]);

        printf("    code lengths:\n");
        u8* ptr = ht.symbols[i];
        for (u8 j=0; j<16; j++) {
            printf("        %d: %d\n", j+1, ht.code_lengths[i][j]);
            if (ht.code_lengths[i][j]>0) {
                printf("            ");
                for (u8 k=0; k<ht.code_lengths[i][j]; k++) {
                    printf("%x ", *ptr++);
                }
                printf("\n");
            }
        }

        if (include_tree) {
            printf("    Tree: \n    ");
            draw_huffman_tree(ht.root[ht.table_class[i]][ht.table_id[i]]);
        }
    }
}

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

void print_sh(ScanHeader& sh) {
    printf("Scan Header: \n");
    printf("-----------  \n");

    for (u8 i=0; i<sh.n_components; i++) {
        printf("Component %d: \n", i);
        printf("    component selector: %d\n", sh.component_selector[i]);
        printf("    dc selector: %d\n", sh.dc_selector[i]);
        printf("    ac selector: %d\n", sh.ac_selector[i]);
    }
    printf("Start spectral: %d\n", sh.start_spectral);
    printf("End spectral: %d\n", sh.end_spectral);
    printf("Approx high: %d\n", sh.approx_high);
    printf("Approx low: %d\n", sh.approx_low);
}

void print_mk(u16 marker) {
    printf("Marker: 0x%X\n", marker);
}

typedef struct {
    u16 width;
    u16 height;
    u8 num_components;
    Markers algorithm;

    QuantizationTables qt;

    FrameHeader fh;

    ScanHeader sh;

    HuffmanTableSpec ht;

    u16 restart_interval;
    u8 dc_pred[4];              // dc_prediction NOTE(alex): move to Quantization Table struct?

    union {
        u32* rgba;
        u32* rgb;
        u8* grey;
    };
} jpeg_t;

u16 peek_2_bytes(u8* v) {
    return v[0]<<8 | v[1];
}

JpegParsingResult parse_StartOfFrame(u8** data, jpeg_t* jpeg) {
    FrameHeader& fh = jpeg->fh;
    u8* ptr = *data;

    u16 length = peek_2_bytes(ptr);
    ptr += 2;

    fh.src_precision = *ptr;
    ptr += 1;
    if (fh.src_precision != 8) {
        return (JpegParsingResult){JPEG_FAIL, "Bytes per color != 8 not supported!"};
    }

    fh.src_height = peek_2_bytes(ptr);
    // if 0 => should be gotten from DNL
    ptr += 2;

    fh.src_width = peek_2_bytes(ptr);
    ptr += 2;

    fh.src_components = *ptr;
    ptr += 1;

    for (u8 i=0; i<fh.src_components; i++) {
        // If components id are 1 2 and 3 => usually YCbCr
        //  else if 0x52, 0x47, 0x42 => RGB (very rare)
        fh.component_id[i] = *ptr;
        ptr++;
        fh.H_sample[i] = (*ptr)>>4;
        fh.V_sample[i] = (*ptr) & 0x0F;
        if (fh.H_sample[i] > 4 || fh.H_sample[i] == 0) {
            return (JpegParsingResult){JPEG_FAIL, "Horizontal sampling factor is not between 1 and 4."};
        } else if (fh.V_sample[i] > 4 || fh.V_sample[i] == 0) {
            return (JpegParsingResult){JPEG_FAIL, "Vertical sampling factor is not between 1 and 4."};
        }
        ptr++;

        fh.QT_selector[i] = *ptr;
        if (fh.QT_selector[i] > 3) {
            return (JpegParsingResult){JPEG_FAIL, "Quantization table selector is not between 0 and 3."};
        }
        ptr++;
    }

    *data = ptr;
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_ApplicationSegment0(u8** data, jpeg_t* jpeg) {
    u8* ptr = *data;
    u16 length = peek_2_bytes(ptr);
    ptr += 2;
    // string8 content = { .buffer = ptr, .size = length-2 };

    *data += length;
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_ApplicationSegment1(u8** data, jpeg_t* jpeg) {
    u8* ptr = *data;
    u16 length = peek_2_bytes(ptr);
    ptr += 2;
    // string8 content = { .buffer = ptr, .size = length-2 };

    *data += length;
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_ApplicationSegment13(u8** data, jpeg_t* jpeg) {
    u8* ptr = *data;
    u16 length = peek_2_bytes(ptr);
    ptr += 2;
    // string8 content = { .buffer = ptr, .size = length-2 };

    *data += length;
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_ApplicationSegment14(u8** data, jpeg_t* jpeg) {
    // This is usually used by adobe encoders
    //
    u8* ptr = *data;
    u16 length = peek_2_bytes(ptr);
    ptr += 2;

    // string8 content = { .buffer = ptr, .size = length-2 };
    // Adobed@
    // Adobed

    // Usually contains a color transform flag at offset 11
    //  value: 0=>RGB, 1=>YCbCr(default), 2=>YCCK

    *data += length;
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_DefineRestartInterval(u8** data, jpeg_t* jpeg) {
    u8* ptr = *data;
    u16 length = peek_2_bytes(ptr);
    ptr += 2;

    // Specifies the number of MCU in the restart interval
    jpeg->restart_interval = peek_2_bytes(ptr);

    *data += length;
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_DefineHuffmanTable(Arena* arena, u8** data, jpeg_t* jpeg) {
    HuffmanTableSpec& ht = jpeg->ht;
    u8* ptr = *data;

    u16 length = peek_2_bytes(ptr);
    ptr += 2;

    // TODO(alex): This has to be done for each huffman table because the segment contains the data for all tables
    //  see itu-t81.pdf page 40
    u8 table_idx = 0;
    while(ptr < *data+length) {
        u8 table_class = *ptr >> 4;                 // 0 DC or lossless  --- 1 AC
        ht.table_class[table_idx] = table_class;
        u8 table_id = (*ptr) & 0x0F;
        ht.table_id[table_idx] = table_id;
        ptr++;

        ht.num_symbols[table_idx] = 0;
        for (u8 i=0; i<16; i++) {
            ht.code_lengths[table_idx][i] = *ptr;
            ht.num_symbols[table_idx] += *ptr;
            ptr++;
        }

        ht.symbols[table_idx] = (u8*)arena_alloc_push_struct(arena, ptr, ht.num_symbols[table_idx]*sizeof(u8));
        ptr += ht.num_symbols[table_idx];

        //==============================
        // Convert table to tree for easier parsing of the entropy stream
        ht.root[table_class][table_id] = (HuffmanNode*)arena_alloc_push_zero_unaligned(arena, sizeof(HuffmanNode));

        HuffmanNode* node = 0;
        u16 code = 0;
        u8* symbol = ht.symbols[table_idx];

        for (u8 i=0; i<16; i++) {
            u8 l = i+1;
            u8 n_codes = ht.code_lengths[table_idx][i];

            // For each code of length i
            for (u16 j=0; j<n_codes; j++, code++, symbol++) {
                node = ht.root[table_class][table_id];

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
        // END OF TABLE
        //==============================

        table_idx++;
    }
    ht.n_tables = table_idx;

    if (ptr != *data + length) {
        return (JpegParsingResult){JPEG_FAIL, "Expected length of DHT and read data does not match."};
    }

    *data = ptr;
    return (JpegParsingResult){JPEG_SUCCESS, "DHT"};
}

JpegParsingResult parse_DefineQuantizationTable(u8** data, jpeg_t* jpeg) {
    QuantizationTables& qt = jpeg->qt;
    u8* ptr = *data;

    u16 length = peek_2_bytes(ptr);
    ptr += 2;

    u8 table_idx = 0;
    qt.n = 0;
    while (ptr<*data+length) {
        qt.n++;

        qt.precision[table_idx] = (*ptr) >> 4;
        qt.id[table_idx]  = (*ptr) & 0x0F;
        ptr++;

        if (qt.precision[table_idx] == 1) {
            return (JpegParsingResult){JPEG_FAIL, "Quantization table precision == 16 bits not supported!"};
        } else if (qt.precision[table_idx] > 1) {
            return (JpegParsingResult){JPEG_FAIL, "Quantization table precision value not supported!"};
        }

        if (qt.id[table_idx]>3) {
            return (JpegParsingResult){JPEG_FAIL, "Quantization table identifier should be between 0 and 3!"};
        }

        for (u8 i=0; i<64; i++) {
            qt.Q[table_idx][i] = *ptr;
            ptr++;
        }
        table_idx++;
    }

    if (ptr != *data + length) {
        return (JpegParsingResult){JPEG_FAIL, "Expected length of DQT and read data does not match."};
    }

    *data = ptr;
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_ScanHeader(u8** data, jpeg_t* jpeg) {
    ScanHeader& sh = jpeg->sh;
    u8* ptr = *data;

    u16 length = peek_2_bytes(ptr);
    ptr += 2;

    // Header
    sh.n_components = *ptr;
    if (sh.n_components > 4 || sh.n_components == 0) {
        return (JpegParsingResult){JPEG_FAIL, "Number of components in a scan must be between 1 and 3."};
    }
    ptr++;

    for (u8 i=0; i<sh.n_components; i++) {
        sh.component_selector[i] = *ptr;
        ptr++;
        sh.dc_selector[i] = (*ptr) >> 4;
        sh.ac_selector[i] = (*ptr) & 0x0F;
        if (sh.dc_selector[i] > 1) {
            return (JpegParsingResult){JPEG_FAIL, "DC coding table must be 0 or 1 for Sequential Baseline DCT."};
        } else if (sh.ac_selector[i] > 1) {
            return (JpegParsingResult){JPEG_FAIL, "AC coding table must be 0 or 1 for Sequential Baseline DCT."};
        }
        ptr++;
    }

    sh.start_spectral = *ptr;
    if (sh.start_spectral != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Baseline DCT must have start spectral = 0"};
    }
    ptr++;

    sh.end_spectral = *ptr;
    if (sh.end_spectral != 63) {
        return (JpegParsingResult){JPEG_FAIL, "Baseline DCT must have end spectral = 0"};
    }
    ptr++;

    sh.approx_high = *ptr >> 4;
    if (sh.approx_high != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Baseline DCT must have approximation high = 0"};
    }
    sh.approx_low  = *ptr & 0x0F;
    if (sh.approx_low != 0) {
        return (JpegParsingResult){JPEG_FAIL, "Baseline DCT must have approximation low = 0"};
    }
    ptr++;

    *data = ptr;
    return (JpegParsingResult){JPEG_SUCCESS, "Successfully read jpeg file!"};
}

void peek_2bytes(BitStream* bs, u16* data) {
    *data = peek_2_bytes(&(bs->data[bs->byte_pos]));
}

void peek_byte(BitStream* bs, u8* byte) {
    *byte = bs->data[bs->byte_pos];
}

bool next_bit(BitStream* bs, u8* bit_out) {
    bool error = false;

    if (&bs->data[bs->byte_pos] >= bs->end) {
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

JpegParsingResult interpret_marker(Arena* arena, u8** data, jpeg_t* jpeg) {
    u8* ptr = *data;
    u16 marker = peek_2_bytes(ptr);
    ptr += 2;
    // Note(alex): ptr automatically moved to the end of the data by each functions
    JpegParsingResult result;
    switch (marker) {
        case ApplicationSegment0: {
            // If contains JFIF => data are YCbCr not RGB (as followinf JFIF standard)
            printf("ApplicationSegment0\n");
            result = parse_ApplicationSegment0(&ptr, jpeg);
        } break;
        case ApplicationSegment1: {
            // Uses EXIF standard
            printf("ApplicationSegment1\n");
            result = parse_ApplicationSegment1(&ptr, jpeg);
        } break;
        case ApplicationSegment13: {
            printf("ApplicationSegment13\n");
            result = parse_ApplicationSegment13(&ptr, jpeg);
        } break;
        case ApplicationSegment14: {
            printf("ApplicationSegment14\n");
            result = parse_ApplicationSegment14(&ptr, jpeg);
        } break;
        case DefineQuantizationTable: {
            printf("DefineQuantizationTable\n");
            result = parse_DefineQuantizationTable(&ptr, jpeg);
        } break;
        case DefineRestartInterval: {
            printf("DefineRestartInterval\n");
            result = parse_DefineRestartInterval(&ptr, jpeg);
        } break;
        case DefineHuffmanTable: {
            printf("DefineHuffmanTable\n");
            result = parse_DefineHuffmanTable(arena, &ptr, jpeg);
        } break;
        case Comment: {
            result = {JPEG_FAIL, "Comment not implemented"};
        } break;
        case DefineNumberOfLines: {
            result = {JPEG_FAIL, "DefineNumberOfLines not implemented"};
        } break;
    }

    if (result.status != JPEG_SUCCESS) {
        return result;
    }
    *data = ptr;
    return result;
}

JpegParsingResult parse_DCDataUnit(BitStream* bs, HuffmanNode* root_node, s16* out) {
    HuffmanNode* node = root_node;

    // Decode the category (code length) using Huffman tree
    u16 code = 0;
    u16 count = 0;
    u8 bit = 0;

    while (!node->is_leaf) {
        next_bit(bs, &bit);
        count++;
        code = (code<<1)|bit;
        if (bit) {
            node = node->right;
            if (node == nullptr){
                printf("CODE: %b  read: %d bits\n", code, count);
                return (JpegParsingResult){JPEG_FAIL, "Failed decoding DC node going right."};
            }
        } else {
            node = node->left;
            if (node == nullptr){
                printf("CODE: %b read: %d bits\n", code, count);
                return (JpegParsingResult){JPEG_FAIL, "Failed decoding DC node going left."};
            }
        }
    }

    u8 category = node->value;

    // If category 0 => value = 0 => early exit
    if (category == 0) {
        *out = 0;
        return (JpegParsingResult){JPEG_SUCCESS, 0};
    }

    // Read that amount of bits
    u16 tmp_value = 0;
    for (u8 i=0; i<category; i++) {
        next_bit(bs, &bit);
        tmp_value = (tmp_value<<1) | bit;
    }

    // Convert to signed value
    if ( tmp_value < (1 << (category-1)) ) {
        *out = (s16)(tmp_value - (1 << category) + 1);
    } else {
        *out = (s16)tmp_value;
    }

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_ACDataUnit(BitStream* bs, HuffmanNode* root_node, u8* run_length, s16* out) {
    HuffmanNode* node = root_node;

    // Huffman decode a byte which contains run length + category
    u8 bit = 0;
    while (!node->is_leaf) {
        next_bit(bs, &bit);
        if (bit) {
            node = node->right;
            if (node == nullptr){
                return (JpegParsingResult){JPEG_FAIL, "Failed decoding AC node going right."};
            }
        } else {
            node = node->left;
            if (node == nullptr){
                return (JpegParsingResult){JPEG_FAIL, "Failed decoding AC node going left."};
            }
        }
    }
    u8 symbol = node->value;
    *run_length = symbol >> 4;
    u8 category = symbol & 0x0F;

    if (category == 0) {
        *out = 0;
        return (JpegParsingResult){JPEG_SUCCESS, 0};
    }

    // Read that amount of bits
    u16 tmp_value = 0;
    for (u8 i=0; i<category; i++) {
        next_bit(bs, &bit);
        tmp_value = (tmp_value<<1) | bit;
    }

    // Convert to signed value
    if ( tmp_value < (1 << (category-1)) ) {        // check if first bit set is 0 => negative
        *out = (s16)(tmp_value - (1 << category) + 1);
    } else {
        *out = (s16)tmp_value;
    }

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_mcu(Arena* arena, BitStream* bs, jpeg_t* jpeg) {
    u8 n_components = jpeg->sh.n_components;

    const u8 DC = 0;
    const u8 AC = 1;
    const u32 PI = 3.1415926535;
    const u8 zigzag[8][8] = {
        { 0,  1,  5,  6, 14, 15, 27, 28},
        { 2,  4,  7, 13, 16, 26, 29, 42},
        { 3,  8, 12, 17, 25, 30, 41, 43},
        { 9, 11, 18, 24, 31, 40, 44, 53},
        {10, 19, 23, 32, 39, 45, 52, 54},
        {20, 22, 33, 38, 46, 51, 55, 60},
        {21, 34, 37, 47, 50, 56, 59, 61},
        {35, 36, 48, 49, 57, 58, 62, 63}
    };

    // This is valid for the whole scan!
    u8 component_idx[4] = {0};
    for (u8 j=0; j<n_components; j++) {
        u8 cs_j = jpeg->sh.component_selector[j];

        for (u8 i=0; i<jpeg->fh.src_components; i++) {
            u8 c_i = jpeg->fh.component_id[i];
            if (c_i == cs_j) {
                component_idx[j] = i;
                break;
            }
        }
    }


    s16 mcu[4][64] = {0};
    s16 idct[4][64] = {0};
    for (u8 i=0; i<n_components; i++) {
        // Get the correct component index in the frame header
        u8 idx = component_idx[i];

        u8 qt_idx = jpeg->fh.QT_selector[idx];
        u8* Q = jpeg->qt.Q[qt_idx];

        u8 dc_table_id = jpeg->sh.dc_selector[i];
        u8 ac_table_id = jpeg->sh.ac_selector[i];
        HuffmanNode* dc_ht_root = jpeg->ht.root[DC][dc_table_id];
        HuffmanNode* ac_ht_root = jpeg->ht.root[AC][ac_table_id];

        for (u8 v=0; v<jpeg->fh.V_sample[idx]; v++) {
            for (u8 h=0; h<jpeg->fh.H_sample[idx]; h++) {
                s16 value;
                JpegParsingResult result = parse_DCDataUnit(bs, dc_ht_root, &value);
                if (result.status != JPEG_SUCCESS) { return result; }
                s16 dc = jpeg->dc_pred[idx] + value;
                jpeg->dc_pred[idx] = dc;
                mcu[idx][0] = dc;

                u8 j = 1;
                while (j<64) {        // and not a marker that would indicate something
                    u8 preceding_zeros = 0;
                    JpegParsingResult result = parse_ACDataUnit(bs, ac_ht_root, &preceding_zeros, &value);
                    s16 ac = value;

                    if (ac == 0 && preceding_zeros == 0) {
                        while (j<64) {
                            mcu[idx][j++] = 0;
                        }
                    } else {
                        for (u8 k=0; k<preceding_zeros; k++) {
                            mcu[idx][j++] = 0;
                        }
                        mcu[idx][j++] = ac;
                    }
                }

                // Dequantize
                for (u8 i=0; i<64; i++) {
                    mcu[idx][i] *= Q[i];
                }

                // IDCT
                // TODO(alex): ZigZag to put at correct indices
                for (u8 y=0; y<8; y++) {
                    for (u8 x=0; x<8; x++) {
                        u8 l = zigzag[y][x];
                        printf("%d ", l);
                        for (u8 u=0; u<8; u++) {
                            f32 Cu = u==0 ? 0.7071067811 : 1;
                            for (u8 v=0; v<8; v++) {
                                f32 Cv = v==0 ? 0.7071067811 : 1;
                                u8 mcu_idx = zigzag[v][u];
                                idct[idx][l] += Cu * Cv * mcu[idx][mcu_idx] * cos((2*x+1)*u*PI*0.0625f) * cos((2*y+1)*v*PI*0.0625f);
                            }
                        }

                        s16 a = idct[idx][l];
                        s16 low = 0;
                        s16 high = 255;
                        s16 t = a < low ? low : a;
                        idct[idx][l] =  t > high ? high : t;
                        // idct[idx][l] = (u8)clamp((s16)(0.25f*)+128, 0, 255);
                    }
                }
            }
        }
    }
    u8 rgb[64][4] = {0};

    // Assume YCbCr
    // Convert to RGB
    for (u8 y=0; y<8; y++) {
        for (u8 x=0; x<8; x++) {
            u8 l = x + y * 8;
            rgb[l][0] = idct[0][l]                        + 1.402   * idct[2][l];
            rgb[l][1] = idct[0][l] - 0.34414 * idct[1][l] - 0.71414 * idct[2][l];
            rgb[l][2] = idct[0][l] + 1.772   * idct[1][l];
        }
    }

    printf("HERE\n");

    // Save

    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_EntropySegment(Arena* arena, u8** data, jpeg_t* jpeg, u8* end_of_data) {
    u8* ptr = *data;
    BitStream bs = {*data, end_of_data-2, 0, 0};
    u16 marker = peek_2_bytes(ptr);

    if (jpeg->restart_interval == 0) {
        return (JpegParsingResult){JPEG_FAIL, "No restart interval is currently unsupported."};
    }

    s8 tmp;
    u16 n = 0;
    while (n++ < jpeg->restart_interval) {
        // Hack to reset all 4 u8 values to 0
        *(u32*)jpeg->dc_pred = 0;

        JpegParsingResult result = parse_mcu(arena, &bs, jpeg);
        if (result.status != JPEG_SUCCESS) { return result; }
    }

    // Go through all the last bits until byte aligned
    u8 bit;
    while (bs.bit_pos != 0) {
        next_bit(&bs, &bit);
    }

    ptr = &bs.data[bs.byte_pos];

    marker = peek_2_bytes(ptr);
    if (!is_restart_marker(marker, &tmp) && !is_interpret_marker(marker) && marker != StartOfScan && marker != EndOfImage) {
        return (JpegParsingResult){JPEG_FAIL, "Invalid marker encountered."};
    }

    *data = ptr;
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult parse_Scan(Arena* persist_arena, Arena* local_arena, u8** data, jpeg_t* jpeg, u8* end_of_data) {
    u8* ptr = *data;

    u16 marker = peek_2_bytes(ptr);
    while (is_interpret_marker(marker) && ptr<end_of_data) {
        JpegParsingResult result = interpret_marker(local_arena, &ptr, jpeg);
        if (result.status != JPEG_SUCCESS) { return result; }
        marker = peek_2_bytes(ptr);
    }

    if (marker != StartOfScan) {
        return (JpegParsingResult){JPEG_FAIL, "Did not find Start of Scan header marker!"};
    }

    ptr += 2;
    JpegParsingResult result = parse_ScanHeader(&ptr, jpeg);
    if (result.status != JPEG_SUCCESS) { return result; }

    s8 restart_id = -1;
    while (true) {
        JpegParsingResult result = parse_EntropySegment(persist_arena, &ptr, jpeg, end_of_data);
        if (result.status != JPEG_SUCCESS) {
            // TODO(alex): Here could check and move forward until next restart marker?
            return result;
        }
        marker = peek_2_bytes(ptr);
        if (is_restart_marker(marker, &restart_id)) {
            ptr += 2;
            continue;
        } else {
            break;
        }
    }

    *data = ptr;
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult decode_frame(Arena* persist_arena, u8** data, jpeg_t* jpeg, u8* end_of_data) {
    LocalArena* local_arena = local_arena_alloc_create();
    u8* ptr = *data;

    // Parse [Tables/misc.] of the frame
    u16 marker = peek_2_bytes(ptr);
    while (is_interpret_marker(marker) && ptr<end_of_data) {
        JpegParsingResult result = interpret_marker(local_arena->arena, &ptr, jpeg);

        if (result.status != JPEG_SUCCESS) { return result; }
        marker = peek_2_bytes(ptr);
    }

    if (ptr>=end_of_data) {
        return (JpegParsingResult){JPEG_FAIL, "Trying to parse beyond the end of file!"};
    }

    if (!is_start_of_frame(marker)) {
        return (JpegParsingResult){JPEG_FAIL, "Did not find start of frame marker!"};
    }

    // Parse Frame header
    ptr += 2;   // Advance from the read marker
    if (marker == StartOfFrame0) {
        jpeg->algorithm = (Markers)marker;
        printf("Parse start of frame!\n");
        JpegParsingResult result = parse_StartOfFrame(&ptr, jpeg);

        if (result.status != JPEG_SUCCESS) { return result; }
    } else {
        return (JpegParsingResult){JPEG_FAIL, "Only StartOfFrame0 marker implemented!"};
    }

    marker = peek_2_bytes(ptr);
    while ((is_interpret_marker(marker) || (marker==StartOfScan)) && ptr<end_of_data) {       // Either DNL between scans or Tables/misc. or Scan header
        JpegParsingResult result = parse_Scan(persist_arena, local_arena->arena, &ptr, jpeg, end_of_data);

        if (result.status != JPEG_SUCCESS) { return result; }
        marker = peek_2_bytes(ptr);
    }

    if (ptr>=end_of_data) {
        return (JpegParsingResult){JPEG_FAIL, "Trying to parse beyond the end of file!"};
    }

    *data = ptr;
    local_arena_alloc_reset(local_arena);
    return (JpegParsingResult){JPEG_SUCCESS, 0};
}

JpegParsingResult decode_jpeg(Arena* persist_arena, string8 data, Image* out) {
    // Note: Current goal is to support baseline DCT 8 bits
    u8* ptr = data.buffer;
    u16 marker = peek_2_bytes(ptr);
    u8* end_of_data = data.buffer + data.size;
    u16 end_marker = peek_2_bytes(end_of_data-2);

    // Get start marker
    if (marker != StartOfImage) {
        printf("First marker is not StartOfImage! Found: 0x%X\n", marker);
        return (JpegParsingResult){JPEG_MISSING_MARKER, "Missing Start Of Image marker"};
    } else if (end_marker != EndOfImage) {
        printf("Last 2 bytes are not EndOfImage! Found: 0x%X\n", end_marker);
        return (JpegParsingResult){JPEG_MISSING_MARKER, "Missing End Of Image marker"};
    }
    // Move to after start of image
    ptr += 2;

    // Decoder_setup
    jpeg_t jpeg = {0};

    JpegParsingResult result = decode_frame(persist_arena, &ptr, &jpeg, end_of_data);

    if (result.status != JPEG_SUCCESS) { return result; }

    marker = peek_2_bytes(ptr);
    if (marker != EndOfImage) {
        return (JpegParsingResult){JPEG_FAIL, "Did not find End Of Image marker!"};
    }

    out->width       = jpeg.fh.src_width;
    out->height      = jpeg.fh.src_height;
    out->components  = jpeg.fh.src_components;
    out->precision   = jpeg.fh.src_precision;
    out->rgb         = jpeg.rgb;                         // works for anything because its a union

    return (JpegParsingResult){JPEG_SUCCESS, "Successfully read jpeg file!"};
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

Image read_image_file(Arena* persist_arena, string8 filename) {
    Image out = {0};
    LocalArena* local_arena = local_arena_alloc_create();

    printf("Reading file: ");
    string_print(filename);
    printf("\n");
    string8 data = read_file(local_arena->arena, filename);

    string8 extension = string_get_file_extension(filename);
    if (extension == ".jpg" || extension == ".jpeg") {
        JpegParsingResult result = decode_jpeg(persist_arena, data, &out);

        if (result.status != JPEG_SUCCESS) {
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
        out = create_missing_image(persist_arena);
    }

    local_arena_alloc_reset(local_arena);
    return out;
}

#endif // LIB_IMAGES_IMPLEMENTATION
