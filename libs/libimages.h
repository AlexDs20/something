#ifndef _LIBIMAGES_H
#define _LIBIMAGES_H

#include "libstring.h"
#include "utils/types.h"
#include "platform/io.h"

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

// itu-t81.pdf page 32
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
    Temporary = 0xFF01,
    Reserved0 = 0xFF02,                 // 0xFF02 to 0xFFBF is reserved
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

    u8 arr[8][8] = {
        { 0,  1,  5,  6, 14, 15, 27, 28},
        { 2,  4,  7, 13, 16, 26, 29, 42},
        { 3,  8, 12, 17, 25, 30, 41, 43},
        { 9, 11, 18, 24, 31, 40, 44, 53},
        {10, 19, 23, 32, 39, 45, 52, 54},
        {20, 22, 33, 38, 46, 51, 55, 60},
        {21, 34, 37, 47, 50, 56, 59, 61},
        {35, 36, 48, 49, 57, 58, 62, 63}
    };
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

u16 swap_endianness(u16 v) {
    u16 out = v<<8 | v>>8;
    return out;
}

struct FrameHeader {
    u8 src_precision;
    u16 src_height;
    u16 src_width;
    u8  src_components;

    u8 component_id[4];
    u8 H_sample[4];
    u8 V_sample[4];
    u8 QT_selector[4];
};

void print_fh(FrameHeader& fh) {
    printf("Frame Header: \n");
    printf("------------  \n");
    printf("Source image: \n");
    printf("    Precision: %d\n", fh.src_precision);
    printf("    Height: %d\n", fh.src_height);
    printf("    Width: %d\n", fh.src_width);
    printf("    Components: %d\n", fh.src_components);

    for (u8 i=0;i<fh.src_components; i++) {
        printf("Component %d:\n", i);
        printf("  id: %d\n", fh.component_id[i]);
        printf("  H sample: %d\n", fh.H_sample[i]);
        printf("  V sample: %d\n", fh.V_sample[i]);
        printf("  QT selector: %d\n", fh.QT_selector[i]);
    }
}

FrameHeader parse_StartOfFrame(u8** data) {
    FrameHeader fh = {0};
    u8* ptr = *data;

    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    fh.src_precision = *ptr;
    ptr += 1;
    if (fh.src_precision != 8) {
        printf("Bytes per color != 8 is not supported, file using: %d\n", fh.src_precision);
        return FrameHeader{0};
    }

    fh.src_height = swap_endianness(*(u16*)ptr);
    // if 0 => should be gotten from DNL
    ptr += 2;

    fh.src_width = swap_endianness(*(u16*)ptr);
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
        ptr++;

        fh.QT_selector[i] = *ptr;
        ptr++;
    }

    // Set the data pointer to the correct place
    // *data += length;
    *data = ptr;
    return fh;
}

void parse_ApplicationSegment0(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;
    // string8 content = { .buffer = ptr, .size = length-2 };

    *data += length;
}

void parse_ApplicationSegment1(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;
    // string8 content = { .buffer = ptr, .size = length-2 };

    *data += length;
}

void parse_ApplicationSegment13(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;
    // string8 content = { .buffer = ptr, .size = length-2 };

    *data += length;
}

void parse_ApplicationSegment14(u8** data) {
    // This is usually used by adobe encoders
    //
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    // string8 content = { .buffer = ptr, .size = length-2 };
    // Adobed@
    // Adobed

    // Usually contains a color transform flag at offset 11
    //  value: 0=>RGB, 1=>YCbCr(default), 2=>YCCK

    *data += length;
}

u16 parse_DefineRestartInterval(u8** data) {
    u8* ptr = *data;
    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    // Specifies the number of MCU in the restart interval
    u16 restart_interval = swap_endianness(*(u16*)ptr);

    *data += length;
    return restart_interval;
}

struct HuffmanNode{
    u16 binary;
    u8 value;
    u8 is_leaf;
    HuffmanNode* left;
    HuffmanNode* right;
    HuffmanNode* parent;
};

struct HuffmanTableSpec {
    u8 n_tables;

    u8 table_class[8];
    u8 table_id[8];
    u8 code_lengths[8][16];
    u8* symbols[8];
    u16 num_symbols[8];

    HuffmanNode* root[8];
};

void print_ht(HuffmanTableSpec& ht) {
    printf("Huffman table spec: \n");
    printf("------------------  \n");

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
                    printf("%d ", *ptr++);
                }
                printf("\n");
            }
        }
    }
}

void print_tree(HuffmanNode* node) {
    if (node->is_leaf) {
        printf(" %d\n", node->value);
    } else {
        print_tree(node->left);
        print_tree(node->right);
    }

}

HuffmanTableSpec parse_DefineHuffmanTable(Arena* arena, u8** data) {
    HuffmanTableSpec ht = {0};
    u8* ptr = *data;

    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    // TODO(alex): This has to be done for each huffman table because the segment contains the data for all tables
    //  see itu-t81.pdf page 40
    u8 table_idx = 0;
    while(ptr < *data+length) {
        ht.table_class[table_idx] = *ptr >> 4;                 // 0 DC or lossless  --- 1 AC
        ht.table_id[table_idx] = (*ptr) & 0x0F;
        ptr++;

        for (u8 i=0; i<16; i++) {
            ht.code_lengths[table_idx][i] = *ptr;
            ht.num_symbols[table_idx] += *ptr;
            ptr++;
        }

        ht.symbols[table_idx] = (u8*)arena_alloc_push_struct(arena, ptr, ht.num_symbols[table_idx]*sizeof(u8));
        ptr += ht.num_symbols[table_idx];

        //==============================
        // Convert table to tree for easier parsing of the entropy stream
        ht.root[table_idx] = (HuffmanNode*)arena_alloc_push_zero_unaligned(arena, sizeof(HuffmanNode));

        HuffmanNode* node = 0;
        u16 code = 0;
        u8* symbol = ht.symbols[table_idx];
        for (u8 i=0; i<16; i++) {
            u8 l = i+1;
            u8 n_codes = ht.code_lengths[table_idx][i];

            node = ht.root[table_idx];
            // For each code of length i
            for (u16 j=0; j<n_codes; j++, code++, symbol++) {
                // Use binary coding to traverse (and create) tree and assign value to node
                for (u16 c=l-1; c<=0; c++) {
                    u8 bit = code >> c;
                    if (bit & 1) { // right
                        if (!node->right) {
                            node->right = (HuffmanNode*)arena_alloc_push_zero_unaligned(arena, sizeof(HuffmanNode));
                            node = node->right;
                        }
                    } else {    // left
                        if (!node->left) {
                            node->left = (HuffmanNode*)arena_alloc_push_zero_unaligned(arena, sizeof(HuffmanNode));
                            node = node->left;
                        }
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
    print_ht(ht);

    *data = ptr;
    return ht;
}

struct ScanHeader {
    u8 n_components;

    u8 component_selector[4];
    u8 dc_selector[4];
    u8 ac_selector[4];

    u8 start_spectral;              // Not used
    u8 end_spectral;                // Not used
    u8 approx_high;                 // Not used
    u8 approx_low;                  // Not used
};

void print_sh(ScanHeader& sh) {
    printf("Scan Header: \n");
    printf("-----------  \n");

    for (u8 i=0; i<sh.n_components; i++) {
        printf("Component %d: \n", i);
        printf("    selector: %d\n", sh.component_selector[i]);
        printf("    dc selector: %d\n", sh.dc_selector[i]);
        printf("    ac selector: %d\n", sh.ac_selector[i]);
    }
    printf("Start spectral: %d\n", sh.start_spectral);
    printf("End spectral: %d\n", sh.end_spectral);
    printf("Approx high: %d\n", sh.approx_high);
    printf("Approx low: %d\n", sh.approx_low);
}

ScanHeader parse_StartOfScan(Arena* persist_arena, u8** data) {
    ScanHeader sh = {0};
    u8* ptr = *data;

    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    // Header
    sh.n_components = *ptr;
    ptr++;

    for (u8 i=0; i<sh.n_components; i++) {
        sh.component_selector[i] = *ptr;
        ptr++;
        sh.dc_selector[i] = (*ptr) >> 4;
        sh.ac_selector[i] = (*ptr) & 0x0F;
        ptr++;
    }

    sh.start_spectral = *ptr;
    ptr++;

    sh.end_spectral = *ptr;
    ptr++;

    sh.approx_high = *ptr >> 4;
    sh.approx_low  = *ptr & 0x0F;
    ptr++;

    *data = ptr;
    return sh;
}

struct QuantizationTables {
    u8 n;
    u8 precision[4];                   // Precision of the Qk: 0 => 8bit, 1 => 16 bit
    u8 destination_identifier[4];      // Specifies where the quantization table to be used in the decoder
    u8 Q[4][64];                       // element in the zigzag ordering of the dct coefficients
};

void print_qt(QuantizationTables& qt) {
    printf("%d Quantization Tables: \n", qt.n);
    printf("-------------------  \n");
    for (u8 i=0; i<qt.n; i++) {
        printf("Precision: %d\n", qt.precision[i]);
        printf("Identifier: %d\n", qt.destination_identifier[i]);
        printf("Q: ");
        for (u8 j=0;j<64; j++) {
            printf(" %d ", qt.Q[i][j]);
        }
        printf("\n");
    }
}

QuantizationTables parse_DefineQuantizationTable(u8** data) {
    QuantizationTables qt = {0};
    u8* ptr = *data;

    u16 length = swap_endianness(*(u16*)ptr);
    ptr += 2;

    u8 table_idx = 0;
    while (ptr<*data+length) {
        qt.n++;

        qt.precision[table_idx] = (*ptr) >> 4;
        qt.destination_identifier[table_idx]  = (*ptr) & 0x0F;
        ptr++;

        if (qt.precision[table_idx] != 0) {
            printf("Quantization table precision != 8 bits not supported!\n");
            return QuantizationTables{0};
        }

        for (u8 i=0; i<64; i++) {
            qt.Q[table_idx][i] = *ptr;
            ptr++;
        }
        table_idx++;
    }

    // This is safer
    // *data += length;
    // but this should work if I didn't fuck up
    *data = ptr;
    return qt;
}

typedef struct {
    u16 width;
    u16 height;
    u8 num_components;

    QuantizationTables qt;

    FrameHeader fh;

    ScanHeader sh;

    HuffmanTableSpec ht;

    u16 restart_interval;

    // jpeg_component_t components[3];

    // uint8_t quant_tables[4][64];
    // uint8_t quant_table_set[4];

    // huffman_table_t huffman_tables[2][4];
    // uint8_t huffman_table_set[2][4];

    // scan_component_t scan_components[3];
    // uint8_t num_scan_components;

    // uint8_t max_h_sampling;
    // uint8_t max_v_sampling;
    // uint16_t mcu_width;
    // uint16_t mcu_height;

    // uint8_t* entropy_data;
    // size_t   entropy_size;

    // int16_t* mcu_blocks;
    // uint8_t* image_data; // final RGB or YCbCr
} jpeg_t;

Image decode_jpeg(Arena* persist_arena, string8 data) {
    Image out = {0};
    LocalArena* local_arena = local_arena_alloc_create();
    u8* end_of_data = (u8*)(data.buffer + data.size);

    // Note: Current goal is to support baseline DCT 8 bits

    jpeg_t jpeg = {0};
    u8* ptr = (u8*)data.buffer;
    u16 marker = swap_endianness(*(u16*)ptr);

    // Get start marker
    if (marker != StartOfImage) {
        printf("First marker is not StartOfImage!\n");
        return out;
    }

    while ( (ptr < end_of_data) && (marker!=EndOfImage) ) {
        // Increase pointer to after the marker
        ptr += 2;
        // TODO(alex): ptr automatically moved to the end of the data by each functions
        bool unknown_marker = false;
        switch (marker) {
            case StartOfImage: {
            } break;
            case ApplicationSegment0: {
                // If contains JFIF => data are YCbCr not RGB (as followinf JFIF standard)
                printf("ApplicationSegment0\n");
                parse_ApplicationSegment0(&ptr);
            } break;
            case ApplicationSegment1: {
                // Uses EXIF standard
                printf("ApplicationSegment1\n");
                parse_ApplicationSegment1(&ptr);
            } break;
            case ApplicationSegment13: {
                printf("ApplicationSegment13\n");
                parse_ApplicationSegment13(&ptr);
            } break;
            case ApplicationSegment14: {
                printf("ApplicationSegment14\n");
                parse_ApplicationSegment14(&ptr);
            } break;
            case DefineQuantizationTable: {
                printf("DefineQuantizationTable\n");
                jpeg.qt = parse_DefineQuantizationTable(&ptr);
            } break;
            case StartOfFrame0: { // This is for baseline DCT
                printf("StartOfFrame0\n");
                jpeg.fh = parse_StartOfFrame(&ptr);
            } break;
            case DefineRestartInterval: {
                printf("DefineRestartInterval\n");
                jpeg.restart_interval = parse_DefineRestartInterval(&ptr);
            } break;
            case DefineHuffmanTable: {
                printf("DefineHuffmanTable\n");
                jpeg.ht = parse_DefineHuffmanTable(local_arena->arena, &ptr);
            } break;
            case StartOfScan: {
                printf("StartOfScan\n");
                jpeg.sh = parse_StartOfScan(persist_arena, &ptr);

                // Actual scan after parsing the header
                u16 bytes = swap_endianness(*(u16*)ptr);
                while (bytes != EndOfImage) {
                    ptr += 1;
                    bytes = swap_endianness(*(u16*)ptr);
                }

            } break;
            default: {
                printf("JPEG MARKER NOT SUPPORTED :: 0x%X\n", marker);
                unknown_marker = true;
            } break;
        }
        if (unknown_marker) break;
        marker = swap_endianness(*(u16*)ptr);
    }

    local_arena_alloc_reset(local_arena);
    return out;
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
        out = decode_jpeg(persist_arena, data);
    } else if (extension == ".png"){
        // out = decode_png(data);
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
