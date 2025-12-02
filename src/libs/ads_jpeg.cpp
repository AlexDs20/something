#include "libs/ads_jpeg.h"
#include <stdio.h>
/*  ==================
 *  JPEG Documentation
 *  ==================
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

// itu-t81.pdf page 32
enum Markers {
    Temporary                     = 0x01,
    Reserved0                     = 0x02,      // 0xFF02 to 0xFFBF is reserved

    StartOfFrame0                 = 0xC0,      // Baseline DCT
    StartOfFrame1                 = 0xC1,      // Extended Sequential DCT
    StartOfFrame2                 = 0xC2,      // Progressive DCT
    StartOfFrame3                 = 0xC3,      // Lossless (sequential)

    DefineHuffmanTable            = 0xC4,

    StartOfFrame5                 = 0xC5,      // Differential sequential DCT
    StartOfFrame6                 = 0xC6,      // Differential progressive DCT
    StartOfFrame7                 = 0xC7,      // Differential lossless (sequential)

    JpegExtensions                = 0xC8,

    StartOfFrame9                 = 0xC9,      // Extended sequential DCT, Arithmetic coding
    StartOfFrame10                = 0xCA,      // Progressive DCT, Arithmetic coding
    StartOfFrame11                = 0xCB,      // Lossless (sequential), Arithmetic coding

    DefineArithmeticCoding        = 0xCC,

    StartOfFrame13                = 0xCD,      // Differential sequential DCT, Arithmetic coding
    StartOfFrame14                = 0xCE,      // Differential progressive DCT, Arithmetic coding
    StartOfFrame15                = 0xCF,      // Differential lossless (sequential), Arithmetic coding

    Restart0                      = 0xD0,      // Restart interval termination
    Restart1                      = 0xD1,
    Restart2                      = 0xD2,
    Restart3                      = 0xD3,
    Restart4                      = 0xD4,
    Restart5                      = 0xD5,
    Restart6                      = 0xD6,
    Restart7                      = 0xD7,

    StartOfImage                  = 0xD8,
    EndOfImage                    = 0xD9,
    StartOfScan                   = 0xDA,
    DefineQuantizationTable       = 0xDB,

    DefineNumberOfLines           = 0xDC,
    DefineRestartInterval         = 0xDD,
    DefineHierarchicalProgression = 0xDE,
    ExpandReferenceComponent      = 0xDF,

    ApplicationSegment0           = 0xE0,      // JFIF jpeg image and AVI1 motion jpeg
    ApplicationSegment1           = 0xE1,      // EXIF metadata, TIFF ifd format, jpeg thumbnail, adobe xmp
    ApplicationSegment2           = 0xE2,      // ICC color profile, flash pix
    ApplicationSegment3           = 0xE3,      // Stereoscopic jpeg images
    ApplicationSegment4           = 0xE4,
    ApplicationSegment5           = 0xE5,
    ApplicationSegment6           = 0xE6,      // NITF lossless profile
    ApplicationSegment7           = 0xE7,
    ApplicationSegment8           = 0xE8,
    ApplicationSegment9           = 0xE9,
    ApplicationSegment10          = 0xEA,      // ActiveObject (multimedia messages/captions)
    ApplicationSegment11          = 0xEB,      // HELIOS JPEG Resources
    ApplicationSegment12          = 0xEC,      // Picture Info, Photoshop Save for Web: Ducky
    ApplicationSegment13          = 0xED,      // Photoshop Save As: IRB, 8BIM, IPTC
    ApplicationSegment14          = 0xEE,
    ApplicationSegment15          = 0xEF,

    JpegExtension0                = 0xF0,
    JpegExtension1                = 0xF1,
    JpegExtension2                = 0xF2,
    JpegExtension3                = 0xF3,
    JpegExtension4                = 0xF4,
    JpegExtension5                = 0xF5,
    JpegExtension6                = 0xF6,
    JpegExtension7                = 0xF7,      // Lossless jpeg
    JpegExtension8                = 0xF8,      // lossless jpeg extension parameters
    JpegExtension9                = 0xF9,
    JpegExtension10               = 0xFA,
    JpegExtension11               = 0xFB,
    JpegExtension12               = 0xFC,
    JpegExtension13               = 0xFD,

    Comment                       = 0xFE,
};

void print_mk(u8 marker) {
    printf("Marker: 0x%02X\n", marker);
}


typedef struct HuffmanNode HuffmanNode;
struct HuffmanNode {
    u8 value;
    u8 is_leaf;
    HuffmanNode* left;
    HuffmanNode* right;
};

typedef struct HuffmanTable HuffmanTable;
struct HuffmanTable {
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

void print_ht(HuffmanTable& ht, bool include_tree=false) {
    printf("Huffman table spec: \n");
    printf("------------------  \n");

    for (u8 i=0; i<4; i++) {
        printf("DC ptr %p:\n", ht.DCroot[i]);
        printf("AC ptr %p:\n", ht.ACroot[i]);

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
    s16* coeff;         // the "raw" decoded coefficient 1 block after each other still in zigzag order
    f32* buffer;        // decoded data
    u8* QT;
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

    // Move to inside the component?
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

    u8 spectral_start;              // Not used for baseline
    u8 spectral_end;                // Not used
    u8 approx_high;                 // Not used
    u8 approx_low;                  // Not used

    // Array of pointer to the components initially created in the FrameHeader
    // The correct DC/AC tables and Quantization tables are set as part of parsing the ScanHeader
    ComponentInfo* components[4];    // Do pointer to component info that is in the frame header
};

void print_sh(ScanHeader& sh) {
    printf("Scan Header: \n");
    printf("-----------  \n");

    for (u8 i=0; i<sh.n_components; i++) {
        printf("Component %d: \n", i);
        printf("    address: %p\n", sh.components[i]);
        // print_ci(sh.components[i]);
    }
    printf("Start spectral: %d\n", sh.spectral_start);
    printf("End spectral: %d\n", sh.spectral_end);
    printf("Approx high: %d\n", sh.approx_high);
    printf("Approx low: %d\n", sh.approx_low);
}

typedef struct Jpeg Jpeg;
struct Jpeg {
    QuantizationTables qt;
    FrameHeader fh;
    ScanHeader sh;
    HuffmanTable ht;

    ComponentInfo component_info[4];
    u8 scan_components_index[4];        // Indices in the ComponentInfo for the current scan and its components (in order)

    u16 restart_interval;
    s16 dc_pred[4];
    u32 current_mcu;

    u8* buffer;
    Markers frame_type;
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
    printf("  size: %ld\n", bs->size);
    printf("  byte: %ld\n", bs->byte_pos);
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
bool skip_nbytes(BitStream* bs, s64 n) {
    bool error = false;
    bs->byte_pos += n;

    if ((bs->byte_pos < 0) | (bs->byte_pos > bs->size-1)) {
        bs->byte_pos -= n;
        error = true;
    }
    return error;
}

u8 read_byte(BitStream* bs) {
    return bs->data[bs->byte_pos++];
}
u16 read_2bytes(BitStream* bs){
    return bs->data[bs->byte_pos++]<<8 | bs->data[bs->byte_pos++];
}

bool next_bit(BitStream* bs, u8* bit_out) {
    bool error = false;

    if (overflow(bs)) {
        printf("Next bit went beyond the bit stream data\n");
        error = true;
        return error;
    }

    u8 byte = bs->data[bs->byte_pos];
    *bit_out = (byte >> (7-bs->bit_pos)) & 0b1;

    // Update
    bs->bit_pos++;
    if (bs->bit_pos == 8) {
        bs->bit_pos = 0;
        bs->byte_pos++;

        // Skip 0xFF00
        if (byte == 0xFF && bs->data[bs->byte_pos] == 0x00) {
            bs->byte_pos++;
        }
    }
    return error;
}

u8 read_bits(BitStream* bs, u8 n) {
    u8 out = 0;
    u8 bit;
    for (u8 i=0; i<n; i++) {
        next_bit(bs, &bit);
        out = (out<<1) | bit;
    }
    return out;
}

u8 peek_byte(BitStream* bs) {
    return bs->data[bs->byte_pos];
}
u16 peek_2bytes(BitStream* bs) {
    return bs->data[bs->byte_pos] << 8 | bs->data[bs->byte_pos+1];
}
// u8 peek_bit(BitStream* bs);

u16 peek_2_bytes(u8* v) {
    return v[0] << 8 | v[1];
}

ImageParsingResult parse_frame_header(Arena* arena, BitStream* bs, Jpeg* jpeg) {
    FrameHeader& fh = jpeg->fh;

    u16 length = read_2bytes(bs);
    length -= 2;

    fh.src_precision = read_byte(bs);
    length--;

    if (fh.src_precision != 8) {
        return (ImageParsingResult){IMAGE_FAIL, "Bytes per color != 8 not supported for DCT baseline or DCT Progressive."};
    }

    fh.src_height = read_2bytes(bs);
    length -= 2;
    if (fh.src_height == 0) {
        // TODO(alex): Go through file until DNL and set the src_height directly here.
        return (ImageParsingResult){IMAGE_FAIL, "NOT IMPLEMENTED: Got src_height of 0 but no support for using DefineNumberOfLines marker yet."};
    }

    fh.src_width = read_2bytes(bs);
    length -= 2;
    if (fh.src_width == 0) {
        return (ImageParsingResult){IMAGE_FAIL, "Missing image with data."};
    }

    fh.src_components = read_byte(bs);
    length--;

    if (fh.src_components == 0) {
        return (ImageParsingResult){IMAGE_FAIL, "Number of components = 0 not supported."};
    }

    if (jpeg->frame_type == StartOfFrame2) {     // Progressive DCT, only support for up to 4 components
        if (fh.src_components > 4) {
            return (ImageParsingResult){IMAGE_FAIL, "Number of components >4 not allowed by spec."};
        }
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
            return (ImageParsingResult){IMAGE_FAIL, "Horizontal sampling factor is not between 1 and 4."};
        } else if (fh.components[i].Vi > 4 || fh.components[i].Vi == 0) {
            return (ImageParsingResult){IMAGE_FAIL, "Vertical sampling factor is not between 1 and 4."};
        }

        fh.Hmax = fh.components[i].Hi > fh.Hmax ? fh.components[i].Hi : fh.Hmax;
        fh.Vmax = fh.components[i].Vi > fh.Vmax ? fh.components[i].Vi : fh.Vmax;

        fh.QT_selector[i] = read_byte(bs);
        length--;
        if (fh.QT_selector[i] > 3) {
            return (ImageParsingResult){IMAGE_FAIL, "Quantization table selector must be between 0 and 3."};
        }
    }

    if (length != 0) {
        return (ImageParsingResult){IMAGE_FAIL, "Expected length of Start of Frame header and read data does not match."};
    }

    // Compute interesting size info related to mcu and components
    fh.n_mcu_width  = ceil((f32)fh.X / (8*(u16)fh.Hmax));
    fh.n_mcu_height = ceil((f32)fh.Y / (8*(u16)fh.Vmax));

    for (u8 i=0; i<fh.src_components; i++) {
        fh.components[i].xi = fh.n_mcu_width  * (8*fh.components[i].Hi);
        fh.components[i].yi = fh.n_mcu_height * (8*fh.components[i].Vi);

        // Note: Only support of 8 bits components
        fh.components[i].buffer = (f32*)arena_alloc_push(arena, fh.components[i].xi * fh.components[i].yi * sizeof (f32));
        fh.components[i].coeff = (s16*)arena_alloc_push_zero(arena, fh.components[i].xi * fh.components[i].yi * sizeof (s16));
        // fh.components[i].buffer = (f32*)arena_alloc_push(arena, fh.components[i].xi * fh.components[i].yi * fh.src_components * sizeof (u8));
    }

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

ImageParsingResult parse_scan_header(BitStream* bs, Jpeg* jpeg) {
    ScanHeader& sh = jpeg->sh;

    u16 length = read_2bytes(bs);
    length -= 2;

    // Header
    sh.n_components = read_byte(bs);
    length--;
    if (sh.n_components > 4 || sh.n_components == 0) {
        return (ImageParsingResult){IMAGE_FAIL, "Number of components in a scan must be between 1 and 4."};
    }

    u8 v;
    u8 dc_selector[4];
    u8 ac_selector[4];
    for (u8 i=0; i<sh.n_components; i++) {
        u8 id_selector = read_byte(bs);
        length--;

        v = read_byte(bs);
        length--;
        dc_selector[i] = v >> 4;
        ac_selector[i] = v & 0x0F;
        if (jpeg->frame_type == StartOfFrame0) {        // Baseline DCT
            if (dc_selector[i] > 1) {
                return (ImageParsingResult){IMAGE_FAIL, "DC coding table must be 0 or 1 for Sequential Baseline DCT."};
            } else if (ac_selector[i] > 1) {
                return (ImageParsingResult){IMAGE_FAIL, "AC coding table must be 0 or 1 for Sequential Baseline DCT."};
            }
        }
        else if (jpeg->frame_type == StartOfFrame2) {   // Progressive DCT
            if (dc_selector[i] > 3) {
                return (ImageParsingResult){IMAGE_FAIL, "DC coding table must be between 0 and 3 for Progressive DCT."};
            } else if (ac_selector[i] > 3) {
                return (ImageParsingResult){IMAGE_FAIL, "AC coding table must be between 0 and 3 for Progressive DCT."};
            }
        }

        // Setup the component informations so that it's complete with QT and AC/DC tables
        for (u8 j=0; j<jpeg->fh.N; j++) {
            if (id_selector == jpeg->fh.components[j].id) {
                sh.components[i] = &jpeg->fh.components[j];

                sh.components[i]->DCHuffmanTable = jpeg->ht.DCroot[dc_selector[i]];
                sh.components[i]->ACHuffmanTable = jpeg->ht.ACroot[ac_selector[i]];

                // Set the correct Quantization table
                u8 qt_id_selector = jpeg->fh.QT_selector[j];
                for (u8 k=0; k<jpeg->qt.n; k++){
                    if (jpeg->qt.id[k] == qt_id_selector) {
                        sh.components[i]->QT = jpeg->qt.Q[k];
                        break;
                    }
                }

                break;
            }
        }
    }

    sh.spectral_start = read_byte(bs);
    length--;

    sh.spectral_end = read_byte(bs);
    length--;

    if (jpeg->frame_type == StartOfFrame0) {
        if (sh.spectral_start != 0) {
            return (ImageParsingResult){IMAGE_FAIL, "Baseline DCT must have start spectral = 0"};
        }
        if (sh.spectral_end != 63) {
            return (ImageParsingResult){IMAGE_FAIL, "Baseline DCT must have end spectral = 63"};
        }
    }
    else if (jpeg->frame_type == StartOfFrame2) {
        if (sh.spectral_end > 63) {
            return (ImageParsingResult){IMAGE_FAIL, "Progressive DCT must have end spectral <= 63"};
        }
        if (sh.spectral_end < sh.spectral_start) {
            return (ImageParsingResult){IMAGE_FAIL, "Progressive DCT must have end spectral smaller than start."};
        }
    }

    // Now that we know the spectral ranges
    // We can check that we have the needed AC/DC tables
    for (u8 i=0; i<sh.n_components; i++) {
        if ((sh.spectral_start == 0) && (jpeg->ht.DCroot[dc_selector[i]] == NULL)) {
            return (ImageParsingResult){IMAGE_FAIL, "Undefined DC table."};
        }

        if ((sh.spectral_end > 0) && (jpeg->ht.ACroot[ac_selector[i]] == NULL)) {
            return (ImageParsingResult){IMAGE_FAIL, "Undefined AC table."};
        }
    }


    v = read_byte(bs);
    length--;
    sh.approx_high = v >> 4;
    sh.approx_low  = v & 0x0F;
    if (jpeg->frame_type == StartOfFrame0) {
        if (sh.approx_high != 0) {
            return (ImageParsingResult){IMAGE_FAIL, "Baseline DCT must have approximation high = 0"};
        }
        if (sh.approx_low != 0) {
            return (ImageParsingResult){IMAGE_FAIL, "Baseline DCT must have approximation low = 0"};
        }
    }
    else if (jpeg->frame_type == StartOfFrame2) {
        if (sh.approx_high > 13) {
            return (ImageParsingResult){IMAGE_FAIL, "Progressive DCT must have approximation high smaller than 13."};
        }
        if (sh.approx_low > 13) {
            return (ImageParsingResult){IMAGE_FAIL, "Progressive DCT must have approximation low smaller than 13."};
        }
    }

    if (length != 0) {
        return (ImageParsingResult){IMAGE_FAIL, "Expected length of Start of Scan header and read data do not match."};
    }

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

ImageParsingResult parse_application_segment_n(BitStream* bs, Jpeg* jpeg) {
    u16 length = peek_2bytes(bs);
    skip_nbytes(bs, length);
    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

ImageParsingResult parse_application_segment_14(BitStream* bs, Jpeg* jpeg) {
    // This is usually used by adobe encoders
    //
    // Adobed@
    // Adobed
    // Usually contains a color transform flag at offset 11
    //  value: 0=>RGB, 1=>YCbCr(default), 2=>YCCK

    u16 length = peek_2bytes(bs);
    skip_nbytes(bs, length);
    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

ImageParsingResult parse_comment(BitStream* bs, Jpeg* jpeg) {
    u16 length = peek_2bytes(bs);
    skip_nbytes(bs, length);
    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

ImageParsingResult parse_define_restart_interval(BitStream* bs, Jpeg* jpeg) {
    u16 length = read_2bytes(bs);
    length -= 2;

    // Specifies the number of MCU in the restart interval
    jpeg->restart_interval = read_2bytes(bs);
    length -= 2;

    if (length != 0) {
        return (ImageParsingResult){IMAGE_FAIL, "Expected length of DefineRestartInterval and read data do not match."};
    }

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

ImageParsingResult parse_define_number_of_lines(BitStream* bs, Jpeg* jpeg) {
    u16 length = read_2bytes(bs);
    length -= 2;

    // Specifies the number of MCU in the restart interval
    jpeg->fh.src_height = read_2bytes(bs);
    length -= 2;

    if (jpeg->fh.src_height == 0) {
        return (ImageParsingResult){IMAGE_FAIL, "NOT IMPLEMENTED: setting the number of lines from the DefineNumberOfLines marker."};
    }

    if (length != 0) {
        return (ImageParsingResult){IMAGE_FAIL, "Expected length of DefineNumberOfLines and read data do not match."};
    }

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

ImageParsingResult parse_define_huffman_table(Arena* arena, BitStream* bs, Jpeg* jpeg) {
    HuffmanTable& ht = jpeg->ht;

    s32 length = (s32)read_2bytes(bs);
    length -= 2;

    u8 v;
    while (length > 0) {
        v = read_byte(bs);
        length--;

        // Get the table type and id
        u8 table_class = v >> 4;                 // 0 DC or lossless  --- 1 AC
        u8 table_id = v & 0x0F;

        if (table_class > 1) {
            return (ImageParsingResult){IMAGE_FAIL, "Huffman table class must be 0 for DC or 1 for AC."};
        }
        if (table_id > 3) {
            return (ImageParsingResult){IMAGE_FAIL, "Huffman table id must be <= 3."};
        }

        // Read actual huffman table code lengths and associated symbols
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
        HuffmanNode* root = (HuffmanNode*)arena_alloc_push_zero(arena, sizeof(HuffmanNode));
        if (table_class == 0) {
            ht.DCroot[table_id] = root;
        } else {
            ht.ACroot[table_id] = root;
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
                            // TODO(alex): Perf: preallocate before the loop so that we don't need to push the pointer everytime?
                            node->right = (HuffmanNode*)arena_alloc_push_zero_unaligned(arena, sizeof(HuffmanNode));
                        }
                        node = node->right;
                    } else {    // left
                        if (!node->left) {
                            // TODO(alex): Perf: preallocate before the loop so that we don't need to push the pointer everytime?
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

    if (length != 0) {
        return (ImageParsingResult){IMAGE_FAIL, "Expected length of DHT and read data does not match."};
    }

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

ImageParsingResult parse_define_quantization_table(BitStream* bs, Jpeg* jpeg) {
    QuantizationTables& qt = jpeg->qt;

    s32 length = (s32)read_2bytes(bs);
    length -= 2;

    while (length > 0) {
        u8 v = read_byte(bs);
        length--;

        u8 precision = v >> 4;
        u8 id = v & 0x0F;

        // Check if the quantization table and its id already exists
        // otherwise add an entry
        u8 idx = jpeg->qt.n;
        {
            for (u8 i=0; i<jpeg->qt.n; i++) {
                if (id == jpeg->qt.id[i]) {
                    idx = i;
                    break;
                }
            }
            if (idx == jpeg->qt.n) {
                jpeg->qt.n++;
            }
        }

        qt.precision[idx] = precision;
        qt.id[idx] = id;

        if (qt.precision[idx] != 0) {
            return (ImageParsingResult){IMAGE_FAIL, "Quantization table precision != 8 bits not supported (yet)."};
        }

        if (qt.id[idx]>3) {
            return (ImageParsingResult){IMAGE_FAIL, "Quantization table identifier should be between 0 and 3."};
        }

        for (u8 i=0; i<64; i++) {
            qt.Q[idx][i] = read_byte(bs);
            length--;

            if (qt.Q[idx][i] == 0) {
                return (ImageParsingResult){IMAGE_FAIL, "Quantization value of 0 not supported (of course)."};
            }
        }
    }

    if (length != 0) {
        return (ImageParsingResult){IMAGE_FAIL, "Expected length of DQT and read data does not match."};
    }

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

bool is_start_of_frame(u8 marker) {
    return (StartOfFrame0 <= marker && marker <= StartOfFrame3) ||
           (StartOfFrame5 <= marker && marker <= StartOfFrame7) ||
           (StartOfFrame9 <= marker && marker <= StartOfFrame11) ||
           (StartOfFrame13 <= marker && marker <= StartOfFrame15);
}

bool is_restart_marker(u8 marker, u8* idx) {
    bool out = true;
    if (Restart0 <= marker && marker <= Restart7) {
        *idx = (*idx+1) & 0x7;
    }
    else {
        out = false;
    }
    return out;
}

bool is_interpret_marker(u8 marker) {
    bool out = false;
    if ( (DefineQuantizationTable == marker) ||
         (DefineHuffmanTable == marker)      ||
         (DefineArithmeticCoding == marker)  ||
         (DefineRestartInterval == marker)   ||
         (Comment == marker)                 ||
         (DefineNumberOfLines == marker)     ||
         (ApplicationSegment0 <= marker && marker <= ApplicationSegment15)) {
    } else{
        out = true;
    }
    return out;
}

ImageParsingResult decode_one_huffman_code(BitStream* bs, HuffmanNode* root, u8* result) {
    HuffmanNode* node = root;

    // Huffman decode category which is the number of bits to decode after
    u8 bit = 0;
    while (!node->is_leaf) {
        next_bit(bs, &bit);
        if (bit) {
            node = node->right;
            if (node == nullptr){
                return (ImageParsingResult){IMAGE_FAIL, "Failed decoding node going right."};
            }
        } else {
            node = node->left;
            if (node == nullptr){
                return (ImageParsingResult){IMAGE_FAIL, "Failed decoding node going left."};
            }
        }
    }

    *result = node->value;

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

s16 decode_n_bits(BitStream* bs, u8 n) {
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

ImageParsingResult parse_dc_data_unit(BitStream* bs, HuffmanNode* root, s16* out) {
    u8 category;
    decode_one_huffman_code(bs, root, &category);

    // If category 0 => value = 0 => early exit
    if (category == 0) {
        *out = 0;
        return (ImageParsingResult){IMAGE_SUCCESS, 0};
    }

    // Read that amount of bits
    *out = decode_n_bits(bs, category);
    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

ImageParsingResult parse_ac_data_unit(BitStream* bs, HuffmanNode* root, u8* run_length, s16* out) {
    u8 symbol;
    // TODO error?
    decode_one_huffman_code(bs, root, &symbol);

    *run_length = symbol >> 4;      // preceeding zeros
    u8 category = symbol & 0x0F;    // Number of bits to read to get the value

    if (category == 0) {
        *out = 0;
        return (ImageParsingResult){IMAGE_SUCCESS, 0};
    }

    *out = decode_n_bits(bs, category);

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

s16 clamps16_0_255(s16 a){
    s16 t = a < 0 ? 0 : a;
    return t > 255 ? 255 : t;
}

f32 clampf32_0_255(f32 a){
    f32 t = a < 0 ? 0: a;
    return t > 255 ? 255 : t;
}

#define SQRT2       1.4142135623730951f
#define SQRT8INV    0.35355339059327373f
#define SQRT2INV    0.7071067811865476f
#define C0          1.000000000000000f
#define C1          0.9807852804032304f    // cos(k*pi/16)
#define C2          0.9238795325112867f
#define C3          0.8314696123025452f
#define C4          0.7071067811865476f
#define C5          0.5555702330196023f
#define C6          0.38268343236508984f
#define C7          0.19509032201612833f

const f32 S1 = C7;
const f32 S2 = C6;
const f32 S3 = C5;
const f32 S4 = C4;
const f32 S5 = C3;
const f32 S6 = C2;
const f32 S7 = C1;

void dct_1d_naive(f32* out, u32 out_stride, f32* in, u32 in_stride) {
    #include "libs/IDCT_Weights.txt"
    for (u8 u=0; u<8; u++) {
        f32 sum = 0;
        for (u8 x=0; x<8; x++) {
            sum += in[x*in_stride] * IDCT_Weights[x][u];
        }
        out[u*out_stride] = sum;
    }
}

void idct_1d_naive(f32* out, u32 out_stride, f32* in, u32 in_stride) {
    #include "libs/IDCT_Weights.txt"
    for (u8 x=0; x<8; x++) {
        for (u8 u=0; u<8; u++) {
            out[x*out_stride] += in[u*in_stride] * IDCT_Weights[x][u];
        }

    }
}

void dct_1d_aan(f32* out, u32 out_stride, f32* in, u32 in_stride) {
    const f32 a1 = C4;
    const f32 a2 = C2 - C6;
    const f32 a3 = C4;
    const f32 a4 = C2 + C6;
    const f32 a5 = C6;

    const f32 b0 =  in[0*in_stride] + in[7*in_stride];
    const f32 b1 =  in[1*in_stride] + in[6*in_stride];
    const f32 b2 =  in[2*in_stride] + in[5*in_stride];
    const f32 b3 =  in[3*in_stride] + in[4*in_stride];
    const f32 b4 = -in[4*in_stride] + in[3*in_stride];
    const f32 b5 = -in[5*in_stride] + in[2*in_stride];
    const f32 b6 = -in[6*in_stride] + in[1*in_stride];
    const f32 b7 = -in[7*in_stride] + in[0*in_stride];

    const f32 c0 = b0 + b3;
    const f32 c1 = b1 + b2;
    const f32 c2 = b1 - b2;
    const f32 c3 = b0 - b3;

    const f32 c4 = -b4 - b5;
    const f32 c5 = b5 + b6;
    const f32 c6 = b6 + b7;
    const f32 c7 = b7;

    const f32 d0 = c0 + c1;
    const f32 d1 = c0 - c1;
    const f32 d2 = c2 + c3;
    const f32 d3 = c3;

    const f32 d4 = c4;
    const f32 d5 = c5;
    const f32 d6 = c6;
    const f32 d7 = c7;

    const f32 d8 = a5*(d4+d6);


    const f32 e0 = d0;
    const f32 e1 = d1;
    const f32 e2 = a1 * d2;
    const f32 e3 = d3;

    const f32 e4 = -d8 - a2 * d4;
    const f32 e5 = a3 * d5;
    const f32 e6 = a4*d6 - d8;
    const f32 e7 = d7;


    const f32 f0 = e0;
    const f32 f1 = e1;
    const f32 f2 = e2 + e3;
    const f32 f3 = e3 - e2;
    const f32 f4 = e4;
    const f32 f5 = e5 + e7;
    const f32 f6 = e6;
    const f32 f7 = e7 - e5;


    const f32 g0 = f0;
    const f32 g1 = f1;
    const f32 g2 = f2;
    const f32 g3 = f3;
    const f32 g4 = f4 + f7;
    const f32 g5 = f5 + f6;
    const f32 g6 = f5 - f6;
    const f32 g7 = f7 - f4;

    // Scaling
    const f32 s0 = SQRT8INV * (1.0f/C0);
    const f32 s1 = 0.25f * (1.0f/C1);
    const f32 s2 = 0.25f * (1.0f/C2);
    const f32 s3 = 0.25f * (1.0f/C3);
    const f32 s4 = 0.25f * (1.0f/C4);
    const f32 s5 = 0.25f * (1.0f/C5);
    const f32 s6 = 0.25f * (1.0f/C6);
    const f32 s7 = 0.25f * (1.0f/C7);

    out[0*out_stride] = s0 * g0;
    out[4*out_stride] = s4 * g1;
    out[2*out_stride] = s2 * g2;
    out[6*out_stride] = s6 * g3;

    out[5*out_stride] = s5 * g4;
    out[1*out_stride] = s1 * g5;
    out[7*out_stride] = s7 * g6;
    out[3*out_stride] = s3 * g7;
}

void idct_1d_aan(f32* out, u32 out_stride, f32* in, u32 in_stride) {
    const f32 s0 = SQRT8INV * (1.0f/C0);
    const f32 s1 = 0.25f * (1.0f/C1);
    const f32 s2 = 0.25f * (1.0f/C2);
    const f32 s3 = 0.25f * (1.0f/C3);
    const f32 s4 = 0.25f * (1.0f/C4);
    const f32 s5 = 0.25f * (1.0f/C5);
    const f32 s6 = 0.25f * (1.0f/C6);
    const f32 s7 = 0.25f * (1.0f/C7);

    const f32 f0 = in[0*in_stride] * s0;
    const f32 f1 = in[4*in_stride] * s4;
    const f32 f2 = in[2*in_stride] * s2;
    const f32 f3 = in[6*in_stride] * s6;
    const f32 f4 = in[5*in_stride] * s5;
    const f32 f5 = in[1*in_stride] * s1;
    const f32 f6 = in[7*in_stride] * s7;
    const f32 f7 = in[3*in_stride] * s3;

    const f32 e0 = f0;
    const f32 e1 = f1;
    const f32 e2 = f2;
    const f32 e3 = f3;
    const f32 e4 = f4 - f7;
    const f32 e5 = f5 + f6;
    const f32 e6 = f5 - f6;
    const f32 e7 = f4 + f7;

    const f32 d0 =      e0;
    const f32 d1 =      e1;
    const f32 d2 = e2 - e3;
    const f32 d3 = e2 + e3;
    const f32 d4 =      e4;
    const f32 d5 = e5 - e7;
    const f32 d6 =      e6;
    const f32 d7 = e5 + e7;


    const f32 c0 =                     d0;
    const f32 c1 =                     d1;
    const f32 c2 =                  C4*d2;
    const f32 c3 =                     d3;
    const f32 c4 = -C6*d6 + d4*(-C2);
    const f32 c5 =                  C4*d5;
    const f32 c6 =  -C6*d4 + d6*(C2);
    const f32 c7 =                     d7;


    const f32 b0 = c0 + c1;
    const f32 b1 = c0 - c1;
    const f32 b2 =      c2;
    const f32 b3 = c2 + c3;
    const f32 b4 =      c4;
    const f32 b5 =      c5;
    const f32 b6 =      c6;
    const f32 b7 =      c7;


    const f32 a0 =  b0 + b3;
    const f32 a1 =  b1 + b2;
    const f32 a2 =  b1 - b2;
    const f32 a3 =  b0 - b3;
    const f32 a4 =      -b4;
    const f32 a5 = -b4 + b5;
    const f32 a6 =  b5 + b6;
    const f32 a7 =  b6 + b7;

    const f32 h0 = a0 + a7;
    const f32 h1 = a1 + a6;
    const f32 h2 = a2 + a5;
    const f32 h3 = a3 + a4;
    const f32 h4 = a3 - a4;
    const f32 h5 = a2 - a5;
    const f32 h6 = a1 - a6;
    const f32 h7 = a0 - a7;

    out[0*out_stride] = h0;
    out[1*out_stride] = h1;
    out[2*out_stride] = h2;
    out[3*out_stride] = h3;
    out[4*out_stride] = h4;
    out[5*out_stride] = h5;
    out[6*out_stride] = h6;
    out[7*out_stride] = h7;


    /*
    const f32 a1 = C4;
    const f32 a2 = C2 - C6;
    const f32 a3 = C4;
    const f32 a4 = C2 + C6;
    const f32 a5 = C6;

    const f32 b0 = f0 + f1 + f2*(a1 + 1) + f3*(1 - a1) + f4*(1 - a5) + f5*(a4 - a5 + 1) + f6*(-a4 + a5 + 1) + f7*(a5 + 1);
    const f32 b1 =        a1*f2 - a1*f3 + f0 - f1 + f4*(-a3 - a5) + f5*(a3 + a4 - a5) + f6*(a3 - a4 + a5) + f7*(-a3 + a5);
    const f32 b2 =        -a1*f2 + a1*f3 + f0 - f1 + f4*(a2 - a3 + a5) + f5*(a3 + a5) + f6*(a3 - a5) + f7*(-a2 - a3 - a5);
    const f32 b3 =                    a5*f5 - a5*f6 + f0 + f1 + f2*(-a1 - 1) + f3*(a1 - 1) + f4*(a2 + a5) + f7*(-a2 - a5);
    const f32 b4 =                   -a5*f5 + a5*f6 + f0 + f1 + f2*(-a1 - 1) + f3*(a1 - 1) + f4*(-a2 - a5) + f7*(a2 + a5);
    const f32 b5 =      -a1*f2 + a1*f3 + f0 - f1 + f4*(-a2 + a3 - a5) + f5*(-a3 - a5) + f6*(-a3 + a5) + f7*(a2 + a3 + a5);
    const f32 b6 =        a1*f2 - a1*f3 + f0 - f1 + f4*(a3 + a5) + f5*(-a3 - a4 + a5) + f6*(-a3 + a4 - a5) + f7*(a3 - a5);
    const f32 b7 =f0 + f1 + f2*(a1 + 1) + f3*(1 - a1) + f4*(a5 - 1) + f5*(-a4 + a5 - 1) + f6*(a4 - a5 - 1) + f7*(-a5 - 1);

    out[0*out_stride] = b0;
    out[1*out_stride] = b1;
    out[2*out_stride] = b2;
    out[3*out_stride] = b3;
    out[4*out_stride] = b4;
    out[5*out_stride] = b5;
    out[6*out_stride] = b6;
    out[7*out_stride] = b7;
    */
}

void dct_1d_llm(f32* out, u32 out_stride, f32* in, u32 in_stride) {
    // TODO: Reduce rotation ops
    const f32 a0 =  in[0*in_stride] + in[7*in_stride];
    const f32 a1 =  in[1*in_stride] + in[6*in_stride];
    const f32 a2 =  in[2*in_stride] + in[5*in_stride];
    const f32 a3 =  in[3*in_stride] + in[4*in_stride];
    const f32 a4 = -in[4*in_stride] + in[3*in_stride];
    const f32 a5 = -in[5*in_stride] + in[2*in_stride];
    const f32 a6 = -in[6*in_stride] + in[1*in_stride];
    const f32 a7 = -in[7*in_stride] + in[0*in_stride];

    const f32 b0 = a0 + a3;
    const f32 b1 = a1 + a2;
    const f32 b2 = a1 - a2;
    const f32 b3 = a0 - a3;
    const f32 b4 =  a4 * C3 + a7 * S3;
    const f32 b7 = -a4 * S3 + a7 * C3;
    const f32 b5 =  a5 * C1 + a6 * S1;
    const f32 b6 = -a5 * S1 + a6 * C1;

    const f32 c0 = b0 + b1;
    const f32 c1 = b0 - b1;
    const f32 c2 = SQRT2 * ( b2 * C6 + b3 * S6);
    const f32 c3 = SQRT2 * (-b2 * S6 + b3 * C6);
    const f32 c4 = b4 + b6;
    const f32 c5 = b7 - b5;
    const f32 c6 = b4 - b6;
    const f32 c7 = b7 + b5;

    const f32 d0 = c0;
    const f32 d1 = c1;
    const f32 d2 = c2;
    const f32 d3 = c3;
    const f32 d4 = c7 - c4;
    const f32 d5 = SQRT2 * c5;
    const f32 d6 = SQRT2 * c6;
    const f32 d7 = c7 + c4;

    // In the article they decided to scale by \sqrt{2} instead of \sqrt{2/N}
    // So I need to add the /sqrt(8)
    out[0*out_stride] = SQRT8INV * d0;
    out[4*out_stride] = SQRT8INV * d1;
    out[2*out_stride] = SQRT8INV * d2;
    out[6*out_stride] = SQRT8INV * d3;
    out[7*out_stride] = SQRT8INV * d4;
    out[3*out_stride] = SQRT8INV * d5;
    out[5*out_stride] = SQRT8INV * d6;
    out[1*out_stride] = SQRT8INV * d7;
}

void idct_1d_llm(f32* out, u32 out_stride, f32* in, u32 in_stride) {
    // TODO: Reduce rotation ops
    const f32 d0 = in[0*in_stride];
    const f32 d1 = in[4*in_stride];
    const f32 d2 = in[2*in_stride];
    const f32 d3 = in[6*in_stride];
    const f32 d4 = in[7*in_stride];
    const f32 d5 = in[3*in_stride];
    const f32 d6 = in[5*in_stride];
    const f32 d7 = in[1*in_stride];

    const f32 c0 = d0;
    const f32 c1 = d1;
    const f32 c2 = d2;
    const f32 c3 = d3;
    const f32 c4 = d7 - d4;
    const f32 c5 = SQRT2 * d5;
    const f32 c6 = SQRT2 * d6;
    const f32 c7 = d7 + d4;

    const f32 b0 = c0 + c1;
    const f32 b1 = c0 - c1;
    const f32 b2 = SQRT2 * ( c2 * C6 - c3 * S6);        // NOTE we need to use inverse rotation (== transposed matrix)
    const f32 b3 = SQRT2 * ( c2 * S6 + c3 * C6);
    const f32 b4 = c4 + c6;
    const f32 b5 = c7 - c5;
    const f32 b6 = c4 - c6;
    const f32 b7 = c5 + c7;

    const f32 a0 = b3 + b0;
    const f32 a1 = b2 + b1;
    const f32 a2 = b1 - b2;
    const f32 a3 = b0 - b3;
    const f32 a4 =  b4 * C3 - b7 * S3;
    const f32 a7 =  b4 * S3 + b7 * C3;
    const f32 a5 =  b5 * C1 - b6 * S1;
    const f32 a6 =  b5 * S1 + b6 * C1;

    out[0*out_stride] = SQRT8INV * (a0 + a7);
    out[1*out_stride] = SQRT8INV * (a1 + a6);
    out[2*out_stride] = SQRT8INV * (a2 + a5);
    out[3*out_stride] = SQRT8INV * (a3 + a4);
    out[4*out_stride] = SQRT8INV * (-a4 + a3);
    out[5*out_stride] = SQRT8INV * (-a5 + a2);
    out[6*out_stride] = SQRT8INV * (-a6 + a1);
    out[7*out_stride] = SQRT8INV * (-a7 + a0);
}

void idct_2d_naive(f32* idct, f32* mcu) {
    f32 Svx[64] = {0};
    for (u8 v=0; v<8; v++) {
        idct_1d_naive(&Svx[v*8], 1, &mcu[v*8], 1);
    }

    for (u8 x=0; x<8; x++) {
        idct_1d_naive(&idct[x], 8, &Svx[x], 8);
    }


    for (u8 l=0; l<64; l++) {
        idct[l] = clampf32_0_255(idct[l] + 128);
    }
}

void idct_2d_llm(f32 idct[64], f32 mcu[64]) {
    f32 Svx[64] = {0};

    for (u8 v=0; v<8; v++) {
        idct_1d_llm(&Svx[v*8], 1, &mcu[v*8], 1);
    }

    for (u8 x=0; x<8; x++) {
        idct_1d_llm(&idct[x], 8, &Svx[x], 8);
    }

    for (u8 l=0; l<64; l++) {
        idct[l] = clampf32_0_255(idct[l] + 128);
    }
}

void idct_2d_aan(f32 idct[64], f32 mcu[64]) {
    f32 Svx[64];

    for (u8 v=0; v<8; v++) {
        idct_1d_aan(&Svx[v*8], 1, &mcu[v*8], 1);
    }

    for (u8 x=0; x<8; x++) {
        idct_1d_aan(&idct[x], 8, &Svx[x], 8);
    }

    for (u8 l=0; l<64; l++) {
        idct[l] = clampf32_0_255(idct[l] + 128);
    }
}

ImageParsingResult parse_baseline_scan(Arena* persist_arena, BitStream* bs, Jpeg* jpeg) {
    u8 expected_restart_id = 0;
    u8 previous;
    u8 marker;

    ImageParsingResult result = {IMAGE_SUCCESS, 0};

    // If only 1 component in the scan => Vi and Hi should be 1
    //  => This means that the number of mcu is not that from the FrameHeader
    u8 Vi[4] = {0};
    u8 Hi[4] = {0};
    u8 n_components = jpeg->sh.n_components;
    u32 n_mcu_width = jpeg->fh.n_mcu_width;
    u32 n_mcu_height = jpeg->fh.n_mcu_height;

    if (n_components == 1) {
        Vi[0] = 1;
        Hi[0] = 1;
        // TODO(alex): Shouldn't this be jpeg->sh.components[0] ==> ScanHeader instead of frameheader
        n_mcu_width =  ceil((f32)jpeg->fh.components[0].xi / 8);
        n_mcu_height = ceil((f32)jpeg->fh.components[0].yi / 8);
    } else {
        for (u8 i=0; i<n_components; i++) {
            Vi[i] = jpeg->sh.components[i]->Vi;
            Hi[i] = jpeg->sh.components[i]->Hi;
        }
    }

    u64 restart_interval;
    u64 n_restart_intervals;

    if (jpeg->restart_interval != 0) {
        restart_interval = jpeg->restart_interval;
        n_restart_intervals = ((n_mcu_width * n_mcu_height) + restart_interval-1) / restart_interval;
    } else {
        n_restart_intervals = 1;
        restart_interval = n_mcu_width * n_mcu_height;
    }

    u64 current_mcu = 0;
    // TODO(alex): Shouldn't this be n_mcu_width*n_mcu_height instead?
    u64 total_number_mcu = jpeg->fh.n_mcu_width * jpeg->fh.n_mcu_height;
    for (u64 i=0; i<n_restart_intervals; i++) {
        // ImageParsingResult result = parse_EntropySegment(persist_arena, bs, jpeg);
        // Reset all 4 s16 values to 0
        jpeg->dc_pred[0] = 0;
        jpeg->dc_pred[1] = 0;
        jpeg->dc_pred[2] = 0;
        jpeg->dc_pred[3] = 0;

        u64 n = 0;
        while (n++ < restart_interval && current_mcu < total_number_mcu) {
            // ImageParsingResult result = parse_mcu(persist_arena, bs, jpeg);
            u32 mcu_block_start_y = (u32)(current_mcu / n_mcu_width);
            u32 mcu_block_start_x = current_mcu - mcu_block_start_y*n_mcu_width;    // = current_mcu % n_mcu_width;

            for (u8 i=0; i<n_components; i++) {
                // u8* Q = jpeg->sh.components[i]->QT;
                HuffmanNode* dc_ht_root = jpeg->sh.components[i]->DCHuffmanTable;
                HuffmanNode* ac_ht_root = jpeg->sh.components[i]->ACHuffmanTable;

                for (u8 v=0; v<Vi[i]; v++) {
                    for (u8 h=0; h<Hi[i]; h++) {
                        //  Identify correctly where to put the data in the component array
                        //  Data format is: block0[64]block1[64],...,blockxi[64,]
                        u32 idx_x = (mcu_block_start_x*Hi[i] + h) * 64;
                        u32 idx_y = (mcu_block_start_y*Vi[i] + v);
                        u64 offset = idx_x + idx_y * (jpeg->sh.components[i]->xi*8);
                        s16* zz_mcu = &jpeg->sh.components[i]->coeff[offset];

                        s16 value;
                        result = parse_dc_data_unit(bs, dc_ht_root, &value);
                        if (result.status != IMAGE_SUCCESS) { return result; }

                        s16 dc = jpeg->dc_pred[i] + value;
                        jpeg->dc_pred[i] = dc;
                        zz_mcu[0] = dc;

                        u8 j = 1;
                        while (j<64) {        // and not a marker that would indicate something
                            u8 preceding_zeros = 0;
                            result = parse_ac_data_unit(bs, ac_ht_root, &preceding_zeros, &value);
                            if (result.status != IMAGE_SUCCESS) { return result; }
                            s16 ac = value;

                            if (ac == 0 && preceding_zeros == 0) {
                                while (j<64) {
                                    zz_mcu[j++] = 0;
                                }
                            } else {
                                for (u8 k=0; k<preceding_zeros; k++) {
                                    zz_mcu[j++] = 0;
                                }
                                zz_mcu[j++] = ac;
                            }
                        }
                    }
                }
            }

            current_mcu++;
        }
        if (result.status != IMAGE_SUCCESS) {
            // TODO(alex): Here could check and move forward until next restart marker?
            return result;
        }

        // Go through all the last bits until byte aligned (bit filling)
        u8 bit;
        while (bs->bit_pos != 0) {
            next_bit(bs, &bit);
        }

        u8 tmp;
        previous = read_byte(bs);
        marker = read_byte(bs);
        if (0xFF != previous) {
            return (ImageParsingResult){IMAGE_FAIL, "Expected a marker."};
        }
        // TODO(alex): check this
        while (marker == 0xFF) {
            printf("TODO(alex): CHECK: DOES THIS HAPPEN!?\n");
            marker = read_byte(bs);
        }
        // if ( !(is_restart_marker(marker, &tmp) | is_interpret_marker(marker) | (StartOfScan == marker) | (EndOfImage == marker)) ) {
        //     return (ImageParsingResult){IMAGE_FAIL, "Invalid marker encountered."};
        // }
        // skip_nbytes(bs, -2);

        // previous = read_byte(bs);
        // marker = read_byte(bs);
        if (is_restart_marker(marker, &expected_restart_id)) {
            continue;
        } else {
            skip_nbytes(bs, -2);
            break;
        }
    }

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

ImageParsingResult decode_progressive_dc(BitStream* bs, Jpeg* jpeg) {
    // - For each restart interval
    // - For each set of MCU in a restart interval e.g. 512 MCU
    // - For each MCU
    // - For each component
    // - For each elem in Hi
    // - For each elem in Vi
    ImageParsingResult result = {IMAGE_SUCCESS, 0};

    u8 Hi[4] = {0};
    u8 Vi[4] = {0};
    u8 n_components = jpeg->sh.n_components;
    u32 n_mcu_width = jpeg->fh.n_mcu_width;
    u32 n_mcu_height = jpeg->fh.n_mcu_height;

    if (n_components == 1) {    // 1 component => no Hi / Vi
        Hi[0] = 1;
        Vi[0] = 1;
        n_mcu_width  = ceil((f32)jpeg->sh.components[0]->xi / 8);
        n_mcu_height = ceil((f32)jpeg->sh.components[0]->yi / 8);
    } else {
        for (u8 i=0; i<n_components; i++) {
            Hi[i] = jpeg->sh.components[i]->Hi;
            Vi[i] = jpeg->sh.components[i]->Vi;
        }
    }
    u64 total_n_mcu = n_mcu_width * n_mcu_height;

    u32 restart_interval_size = jpeg->restart_interval != 0 ? jpeg->restart_interval : total_n_mcu;
    u32 restart_interval_num  = ceil((f32)total_n_mcu / restart_interval_size);

    u64 processed_mcu = 0;
    for (u32 ri_idx=0; ri_idx<restart_interval_num; ri_idx++, processed_mcu+=restart_interval_size) {
        s16 dc_pred[4] = {0};

        for (u32 mcu_idx=0; mcu_idx<restart_interval_size; mcu_idx++) {
            if (processed_mcu + mcu_idx >= total_n_mcu) {
                break;
            }
            u32 mcu_block_start_y = (u32)((processed_mcu + mcu_idx) / n_mcu_width);
            u32 mcu_block_start_x =       (processed_mcu + mcu_idx) - mcu_block_start_y*n_mcu_width;    // = current_mcu % n_mcu_width;

            for (u8 comp_idx=0; comp_idx<n_components; comp_idx++) {
                HuffmanNode* dc_ht_root = jpeg->sh.components[comp_idx]->DCHuffmanTable;

                for (u8 vi=0; vi<Vi[comp_idx]; vi++) {
                    for (u8 hi=0; hi<Hi[comp_idx]; hi++) {
                        // Lots of stuff just to write to the write spot...
                        u32 idx_x = (mcu_block_start_x*Hi[comp_idx] + hi) * 64;
                        u32 idx_y = (mcu_block_start_y*Vi[comp_idx] + vi);
                        u64 offset = idx_x + idx_y * (jpeg->sh.components[comp_idx]->xi*8);
                        s16* zz_mcu = &jpeg->sh.components[comp_idx]->coeff[offset];

                        // Decode DC
                        if (jpeg->sh.approx_high == 0) {    // First time decoding DC
                            s16 value;
                            result = parse_dc_data_unit(bs, dc_ht_root, &value);
                            if (result.status != IMAGE_SUCCESS) { return result; }
                            dc_pred[comp_idx] += value;

                            zz_mcu[0] = dc_pred[comp_idx] << jpeg->sh.approx_low;
                        } else {
                            u8 bit;
                            bool FIXME_error = next_bit(bs, &bit);
                            zz_mcu[0] |= (bit << jpeg->sh.approx_low);
                        }
                    }
                }
            }
        }
        if (result.status != IMAGE_SUCCESS) {
            // TODO(alex): Here could check and move forward until next restart marker?
            return result;
        }

        // Go through all the last bits until byte aligned (bit filling)
        u8 bit;
        while (bs->bit_pos != 0) {
            next_bit(bs, &bit);
        }

        u8 tmp;
        u8 previous = read_byte(bs);
        u8 marker = read_byte(bs);
        if (0xFF != previous) {
            return (ImageParsingResult){IMAGE_FAIL, "Expected a marker."};
        }
        // TODO(alex): check this
        while (marker == 0xFF) {
            printf("TODO(alex): CHECK: DOES THIS HAPPEN!?\n");
            marker = read_byte(bs);
        }
        if (is_restart_marker(marker, &tmp)) {
            continue;
        } else {
            skip_nbytes(bs, -2);
            break;
        }
    }

    return result;
}

ImageParsingResult decode_progressive_ac(BitStream* bs, Jpeg* jpeg) {
    ImageParsingResult result = {IMAGE_SUCCESS, 0};

    u32 n_mcu_width = ceil((f32)jpeg->sh.components[0]->xi / 8);
    u32 n_mcu_height = ceil((f32)jpeg->sh.components[0]->yi / 8);
    u64 total_n_mcu = n_mcu_width * n_mcu_height;

    u32 restart_interval_size = jpeg->restart_interval != 0 ? jpeg->restart_interval : total_n_mcu;
    u32 restart_interval_num  = ceil((f32)total_n_mcu / restart_interval_size);

    HuffmanNode* ac_ht_root = jpeg->sh.components[0]->ACHuffmanTable;

    u64 processed_mcu = 0;
    for (u32 ri_idx=0; ri_idx<restart_interval_num; ri_idx++, processed_mcu+=restart_interval_size) {
        s16 dc_pred[4] = {0};

        for (u32 mcu_idx=0; mcu_idx<restart_interval_size; mcu_idx++) {
            if (processed_mcu + mcu_idx >= total_n_mcu) {
                break;
            }

            u32 mcu_block_start_y = (u32)((processed_mcu + mcu_idx) / n_mcu_width);
            u32 mcu_block_start_x =       (processed_mcu + mcu_idx) - mcu_block_start_y*n_mcu_width;    // = current_mcu % n_mcu_width;

            // Lots of stuff just to write to the write spot...
            u32 idx_x = (mcu_block_start_x*1 + 0) * 64;
            u32 idx_y = (mcu_block_start_y*1 + 0);
            u64 offset = idx_x + idx_y * (jpeg->sh.components[0]->xi*8);
            s16* zz_mcu = &jpeg->sh.components[0]->coeff[offset];

            // Decode AC
            if (jpeg->sh.approx_high == 0) {    // First time decoding AC
                s16 ac;
                u8 preceding_zeros;

                for (u8 idx=jpeg->sh.spectral_start; idx<=jpeg->sh.spectral_end; idx++) {
                    result = parse_ac_data_unit(bs, ac_ht_root, &preceding_zeros, &ac);
                    if (result.status != IMAGE_SUCCESS) { return result; }

                    if (ac == 0) {
                        // Fill 16 zeros
                        if (preceding_zeros==0xF) {
                            // Write 16 zeros but it should not be needed if we default initialize to 0
                            // idx += 0xF;
                            for (u8 j=0; j<0xF; j++) {
                                zz_mcu[idx++] = 0;
                            }
                            continue;
                        }
                        // Fill rest of band with 0
                        else if (preceding_zeros==0) {
                            // Assumes zero initialized
                            break;
                        }
                        // Skip several blocks of bands which are 0
                        else {
                            // Get amount of blocks to skip
                            u64 skips = (1<<preceding_zeros) - 1;
                            skips += read_bits(bs, preceding_zeros);
                            mcu_idx += skips;
                            break;
                        }
                    }
                    else {
                        for (u8 j=0; j<preceding_zeros; j++) {
                            zz_mcu[idx++] = 0;
                        }
                        zz_mcu[idx] = ac << jpeg->sh.approx_low;
                        continue;
                    }
                }

            } else {                            // Subsequent
                break;
                // TODO
                // zz_mcu[0] |= (bit << jpeg->sh.approx_low);
            }
        }
        if (result.status != IMAGE_SUCCESS) {
            // TODO(alex): Here could check and move forward until next restart marker?
            return result;
        }

        // Go through all the last bits until byte aligned (bit filling)
        u8 bit;
        while (bs->bit_pos != 0) {
            next_bit(bs, &bit);
        }

        u8 tmp;
        u8 previous = read_byte(bs);
        u8 marker = read_byte(bs);
        if (0xFF != previous) {
            return (ImageParsingResult){IMAGE_FAIL, "Expected a marker."};
        }
        // TODO(alex): check this
        while (marker == 0xFF) {
            printf("TODO(alex): CHECK: DOES THIS HAPPEN!?\n");
            marker = read_byte(bs);
        }
        if (is_restart_marker(marker, &tmp)) {
            continue;
        } else {
            skip_nbytes(bs, -2);
            break;
        }
    }

    return result;
}

ImageParsingResult parse_progressive_scan(Arena* persist_arena, BitStream* bs, Jpeg* jpeg) {
    ImageParsingResult result = {IMAGE_SUCCESS, 0};

    if (jpeg->sh.spectral_end == 0) {       // DC
        result = decode_progressive_dc(bs, jpeg);
    }
    else if (jpeg->sh.spectral_start > 0) { // AC
        if (jpeg->sh.approx_high == 0) {
            result = decode_progressive_ac(bs, jpeg);
        }
    } else {
        return (ImageParsingResult){IMAGE_FAIL, "Unexpected combination of parameters for a scan during progressive decoding."};
    }
#if 1
            u8 previous = read_byte(bs);
            u8 marker = read_byte(bs);
            while (true) {
                if (previous == 0xFF && (marker != 0x00 && (marker < Restart0 || marker>Restart7)) ) {
                    printf("MARKER: 0x%02X%02X\n", previous, marker);
                    skip_nbytes(bs, -2);
                    bs->bit_pos = 0;
                    break;
                }
                previous = marker;
                marker = read_byte(bs);
            }
#endif
    return result;
}

ImageParsingResult parse_scans(Arena* persist_arena, Arena* local_arena, BitStream* bs, Jpeg* jpeg) {
    u8 previous = read_byte(bs);
    u8 marker   = read_byte(bs);
    ImageParsingResult result;

    while (EndOfImage != marker) {
        print_mk(marker);
        if (StartOfScan == marker) {
            result = parse_scan_header(bs, jpeg);
            if (result.status != IMAGE_SUCCESS) { return result; }

            printf("Parsing scan\n");
            printf("Spectral band: [%d,%d] Approx: [%d,%d] Components: %d Cid: %p\n", jpeg->sh.spectral_start, jpeg->sh.spectral_end, jpeg->sh.approx_high, jpeg->sh.approx_low, jpeg->sh.n_components, jpeg->sh.components[0]);
            if (StartOfFrame0 == jpeg->frame_type) {
                result = parse_baseline_scan(persist_arena, bs, jpeg);
            } else if (StartOfFrame2 == jpeg->frame_type) {
                result = parse_progressive_scan(persist_arena, bs, jpeg);
            }
            printf("Done parsing scan\n");
        }
        else if (DefineQuantizationTable == marker) {
            result = parse_define_quantization_table(bs, jpeg);
        }
        else if (DefineHuffmanTable == marker) {
            result = parse_define_huffman_table(local_arena, bs, jpeg);
        }
        else if (DefineRestartInterval == marker) {
            result = parse_define_restart_interval(bs, jpeg);
        }
        else if (DefineNumberOfLines == marker) {
            result = parse_define_number_of_lines(bs, jpeg);
        }
        else if (ApplicationSegment0 <= marker && marker <= ApplicationSegment14) {
            result = parse_application_segment_n(bs, jpeg);
        }
        else if (Comment == marker) {
            result = parse_comment(bs, jpeg);
        }
        else if (0xFF == marker) {
            marker = read_byte(bs);
            continue;
        }
        else {
            return (ImageParsingResult){IMAGE_FAIL, "Unidentified marker."};
        }
        if (result.status != IMAGE_SUCCESS) { return result; }

        // Read next marker
        previous = read_byte(bs);
        if (previous != 0xFF) {
            print_mk(marker);
            return (ImageParsingResult){IMAGE_FAIL, "Expected a marker."};
        }
        marker = read_byte(bs);
    }

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}

// TODO(alex): This should not use Arena and string8 but standard C types.
ImageParsingResult decode_jpeg(Arena* persist_arena, string8 data, Image* out) {
    // Note: Current goal is to support baseline DCT 8 bits
    BitStream bs = {
        .data = data.buffer,
        .size = data.size,
        .byte_pos = 0,
        .bit_pos = 0
    };
    u8 previous = read_byte(&bs);
    u8 marker = read_byte(&bs);

    if (previous != 0xFF) {
        return (ImageParsingResult){IMAGE_FAIL, "Did not start with a marker."};
    }
    else if (marker != StartOfImage) {
        return (ImageParsingResult){IMAGE_FAIL, "Missing Start Of Image marker."};
    }
    else if (bs.data[bs.size-2] != 0xFF) {
        return (ImageParsingResult){IMAGE_FAIL, "Last 2 bytes are not a marker."};
    }
    else if (bs.data[bs.size-1] != EndOfImage) {
        return (ImageParsingResult){IMAGE_FAIL, "Last 2 bytes are not the EndOfImage marker."};
    }

    // Decoder_setup
    Jpeg jpeg = {0};

    //==============================
    //  Decoding
    {
        ImageParsingResult result;
        LocalArena* local_arena = local_arena_alloc_create();

        // Parse [Tables/misc.] of the frame
        previous = read_byte(&bs);
        if (previous != 0xFF) {
            return (ImageParsingResult){IMAGE_FAIL, "Expected a marker."};
        }
        marker = read_byte(&bs);
        while (EndOfImage != marker) {
            if (is_start_of_frame(marker)) {       // Not optimal to have it this early in the if list...
                break;
            }
            else if (DefineQuantizationTable == marker) {
                result = parse_define_quantization_table(&bs, &jpeg);
            }
            else if (DefineHuffmanTable == marker) {
                result = parse_define_huffman_table(local_arena->arena, &bs, &jpeg);
            }
            else if (DefineArithmeticCoding == marker) {
                result = (ImageParsingResult){IMAGE_FAIL, "Arithmetic Coding not supported."};
            }
            else if (DefineRestartInterval == marker) {
                result = parse_define_restart_interval(&bs, &jpeg);
            }
            else if (DefineNumberOfLines == marker) {
                result = parse_define_number_of_lines(&bs, &jpeg);
            }
            else if (ApplicationSegment0 <= marker && marker <= ApplicationSegment14) {
                result = parse_application_segment_n(&bs, &jpeg);
            }
            else if (Comment == marker) {
                result = parse_comment(&bs, &jpeg);
            }
            else if (0xFF == marker) {
                marker = read_byte(&bs);
                continue;
            }
            else {
                result = (ImageParsingResult){IMAGE_FAIL, "Reached an unknown marker."};
            }
            if (result.status != IMAGE_SUCCESS) { return result; }

            // Read next marker
            previous = read_byte(&bs);
            if (previous != 0xFF) {
                return (ImageParsingResult){IMAGE_FAIL, "Expected a marker."};
            }
            marker = read_byte(&bs);
        }

        if (EndOfImage == marker) {
            return (ImageParsingResult){IMAGE_FAIL, "Reached EndOfImage before parsing a StartOfFrame header!"};
        }
        else if (!is_start_of_frame(marker)) {
            return (ImageParsingResult){IMAGE_FAIL, "Did not find start of frame marker! (We should never get here...)"};
        }

        //  PARSE FRAME
        {
            // FrameHeader
            if (marker == StartOfFrame0 || marker == StartOfFrame2) {      // Baseline DCT or Progressive
                jpeg.frame_type = (Markers)marker;
                result = parse_frame_header(local_arena->arena, &bs, &jpeg);
                if (result.status != IMAGE_SUCCESS) { return result; }
            }
            else {
                return (ImageParsingResult){IMAGE_FAIL, "Only StartOfFrame0 marker implemented!"};
            }

            // TODO(alex): Allocate precisely the right amount and use u8* regardless
            jpeg.buffer = (u8*)arena_alloc_push(persist_arena, jpeg.fh.src_width*jpeg.fh.src_height*4*sizeof(u8));

            // Parse all Scans
            result = parse_scans(persist_arena, local_arena->arena, &bs, &jpeg);
            if (result.status != IMAGE_SUCCESS) { return result; }
        }

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

        for (u8 i=0; i<jpeg.fh.src_components; i++) {
            s16* comp = jpeg.fh.components[i].coeff;
            u8* Q = jpeg.fh.components[i].QT;

            u32 n_du_width =  ceil((f32)jpeg.fh.components[i].xi / 8);
            u32 n_du_height = ceil((f32)jpeg.fh.components[i].yi / 8);

            // If I put them contiguous in memory
            for (u32 block_id_y=0; block_id_y<n_du_height; block_id_y++) {
                for (u32 block_id_x=0; block_id_x<n_du_width; block_id_x++) {
                    u64 block_id = block_id_y * n_du_width + block_id_x;

                    s16* coeff = &comp[block_id * 64];
                    f32 mcu[64] = {0};
                    f32 idct[64] = {0};

                    // Dequantize and Unzigzag
                    for (u8 l=0; l<64; l++) {
                        mcu[unzigzag[l]] = coeff[l] * Q[l];
                    }

                    // IDCT
                    idct_2d_aan(idct, mcu);

                    // Write data at correct place
                    u32 idx_x = block_id_x * 8;
                    u32 idx_y = block_id_y * 8;
                    u32 base_offset = idx_y * jpeg.sh.components[i]->xi + idx_x;
                    for (u8 y=0; y<8; y++) {
                        for (u8 x=0; x<8; x++) {
                            u64 linear_index = base_offset + y * jpeg.sh.components[i]->xi + x;
                            u16 l = x + y*8;
                            jpeg.sh.components[i]->buffer[linear_index] = idct[l];
                        }
                    }
                }
            }
        }

        // Assume YCbCr
        // Convert to RGB
        int tmp = 0x000000FF;
        bool is_little_endian = (*(u8*)(&tmp) == 0xFF);
        if (3 == jpeg.fh.src_components) {
            f32* comp0 = jpeg.sh.components[0]->buffer;
            f32* comp1 = jpeg.sh.components[1]->buffer;
            f32* comp2 = jpeg.sh.components[2]->buffer;

            u64 l0, l0_x, l0_y;
            u64 l1, l1_x, l1_y;
            u64 l2, l2_x, l2_y;

            u8 l0_x_ratio, l0_y_ratio;
            u8 l1_x_ratio, l1_y_ratio;
            u8 l2_x_ratio, l2_y_ratio;

            l0_x_ratio = jpeg.fh.Hmax / jpeg.fh.components[0].Hi;
            l0_y_ratio = jpeg.fh.Vmax / jpeg.fh.components[0].Vi;

            l1_x_ratio = jpeg.fh.Hmax / jpeg.fh.components[1].Hi;
            l1_y_ratio = jpeg.fh.Vmax / jpeg.fh.components[1].Vi;

            l2_x_ratio = jpeg.fh.Hmax / jpeg.fh.components[2].Hi;
            l2_y_ratio = jpeg.fh.Vmax / jpeg.fh.components[2].Vi;

            for (u32 y=0; y<jpeg.fh.src_height; y++) {
                l0_y = y / l0_y_ratio;
                l1_y = y / l1_y_ratio;
                l2_y = y / l2_y_ratio;

                for (u32 x=0; x<jpeg.fh.src_width; x++) {
                    l0_x = x / l0_x_ratio;
                    l1_x = x / l1_x_ratio;
                    l2_x = x / l2_x_ratio;

                    u64 l = y * jpeg.fh.src_width + x;
                    l0 = (l0_y*jpeg.fh.components[0].xi) + l0_x;
                    l1 = (l1_y*jpeg.fh.components[1].xi) + l1_x;
                    l2 = (l2_y*jpeg.fh.components[2].xi) + l2_x;

                    f32 r = comp0[l0]                                 + 1.402   * (comp2[l2] - 128);
                    f32 g = comp0[l0] - 0.34414 * (comp1[l1] - 128)   - 0.71414 * (comp2[l2] - 128);
                    f32 b = comp0[l0] + 1.772   * (comp1[l1] - 128);

                    u8 red = (u8)clampf32_0_255(r+0.5);
                    u8 green = (u8)clampf32_0_255(g+0.5);
                    u8 blue = (u8)clampf32_0_255(b+0.5);
                    u32* p = (u32*)(jpeg.buffer+l*4);
                    if (is_little_endian) {
                        *p = 0xFF << 24 | blue << 16 | green << 8 | red;
                    } else {
                        *p = red<<24 | green<<16 | blue<<8 | 0xFF;
                    }
                }
            }
        } else if (1 == jpeg.fh.src_components) {
            f32* comp0 = jpeg.sh.components[0]->buffer;
            for (u32 y=0; y<jpeg.fh.src_height; y++) {
                for (u32 x=0; x<jpeg.fh.src_width; x++) {
                    u64 l = y * jpeg.fh.src_width + x;
                    jpeg.buffer[l*4 + 0] = (u8)clampf32_0_255(comp0[l]);
                    jpeg.buffer[l*4 + 1] = (u8)clampf32_0_255(comp0[l]);
                    jpeg.buffer[l*4 + 2] = (u8)clampf32_0_255(comp0[l]);
                    jpeg.buffer[l*4 + 3] = 255;
                }
            }
        }

        local_arena_alloc_reset(local_arena);
        result = (ImageParsingResult){IMAGE_SUCCESS, 0};

        if (result.status != IMAGE_SUCCESS) { return result; }
    }
    //  End Decoding
    //==============================

    out->width       = jpeg.fh.src_width;
    out->height      = jpeg.fh.src_height;
    out->components  = jpeg.fh.src_components;
    out->precision   = jpeg.fh.src_precision;
    out->buffer      = (u32*)jpeg.buffer;

    return (ImageParsingResult){IMAGE_SUCCESS, 0};
}
