#include <stdio.h>
#include <time.h>
#include "gf_profiling.c"
#include "tests/macros.h"

#include "src/utils/defines.h"
#include "memory/allocators.h"
Arena* arena = arena_alloc_create_zero(1*GiB);

#include "tests/test_platform_io.h"
#include "tests/test_decode_jpeg.h"
#include "tests/test_ads_string.h"
#include "tests/test_ads_model_loader.h"

typedef struct {
    const char* name;
    int (*function)(void);
} test_entry;

test_entry tests[] = {
#if 0
    //==============================
    // ADS_IMAGE
    // {"JPEG_baseline_rgb", test_read_baseline_rgb},
    // {"JPEG_baseline_gray", test_read_baseline_gray},
    // {"JPEG_baseline_gray_2", test_read_baseline_gray_2},

    //==============================
    // ADS_STRING
    {"test_string_init_empty", test_string_init_empty},
    {"test_string_init_fmt", test_string_init_fmt},
    {"test_string_init_sv", test_string_init_sv},
    {"test_string_init_cstr", test_string_init_cstr},
    {"test_string_init_buffer", test_string_init_buffer},

    {"test_string_append_fmt", test_string_append_fmt},
    {"test_string_append_sv", test_string_append_sv},
    {"test_string_append_string", test_string_append_string},
    {"test_string_append_cstr", test_string_append_cstr},
    {"test_string_append_buffer", test_string_append_buffer},
    {"test_string_append_char", test_string_append_char},

    {"test_string_prepend_fmt", test_string_prepend_fmt},
    {"test_string_prepend_sv", test_string_prepend_sv},
    // {"test_string_prepend_string", test_string_prepend_string},
    // {"test_string_prepend_cstr", test_string_prepend_cstr},
    // {"test_string_prepend_buffer", test_string_prepend_buffer},
    // {"test_string_prepend_char", test_string_prepend_char},

    {"test_string_insert_fmt", test_string_insert_fmt},
    {"test_string_insert_sv", test_string_insert_sv},
    // {"test_string_insert_buffer", test_string_insert_buffer},

    {"test_string_overwrite_fmt", test_string_overwrite_fmt},
    {"test_string_overwrite_sv", test_string_overwrite_sv},
    // {"test_string_overwrite_buffer", test_string_overwrite_buffer},

    {"test_string_erase_and_insert_sv", test_string_erase_and_insert_sv},
    {"test_string_erase_and_insert_fmt", test_string_erase_and_insert_fmt},

    {"test_string_replace_all", test_string_replace_all},

    // {"test_string_erase", test_string_erase},
    // {"test_string_clear", test_string_clear},
    // {"test_string_deep_copy", test_string_deep_copy},
    {"test_string_reserve", test_string_reserve},
    {"test_string_shrink_to_fit", test_string_shrink_to_fit},


    {"test_sv_from_buffer", test_sv_from_buffer},
    {"test_sv_from_cstr", test_sv_from_cstr},
    // {"test_sv_from_string", test_sv_from_string},

    {"test_sv_slice_sv", test_sv_slice_sv},
    {"test_sv_slice_string", test_sv_slice_string},
    {"test_sv_truncate_front", test_sv_truncate_front},
    {"test_sv_truncate_back", test_sv_truncate_back},
    {"test_sv_trim_front", test_sv_trim_front},
    {"test_sv_trim_back", test_sv_trim_back},
    {"test_sv_chop_by_delim_sv", test_sv_chop_by_delim_sv},

    {"test_sv_equal", test_sv_equal},
    {"test_sv_compare", test_sv_compare},
    {"test_sv_starts_with", test_sv_starts_with},
    {"test_sv_ends_with", test_sv_ends_with},
    {"test_sv_find", test_sv_find},
    {"test_sv_rfind", test_sv_rfind},

    {"test_sv_file_extension", test_sv_file_extension},

    {"test_sv_parse_u32", test_sv_parse_u32},
    {"test_sv_parse_s32", test_sv_parse_s32},
#endif
    {"test_parser_f32", test_parser_f32},

    //==============================
    // ADS_MODEL_LOADER
    {"test_model_read", test_model_read},

    //==============================
    // PLATFORM_IO
    // {"test_read_complete_file", test_read_complete_file},
};

int main() {
    int failed = 0;

    for (int i=0; i<ARRAY_SIZE(tests); i++) {
        printf("%s started (%d/%d)", tests[i].name, i+1, ARRAY_SIZE(tests));
        clock_t start = clock();
        int r = tests[i].function();
        arena_alloc_reset(arena);
        clock_t stop = clock();
        printf(" (%.4fs) ", (double)(stop - start) / CLOCKS_PER_SEC);
        if (r!=0) {
            failed++;
            printf("\tFailed at line: %d\n", r);
        }
        printf("\n");
    }

    printf("Total failed tests: %d/%d\n", failed, ARRAY_SIZE(tests));

    return 0;
}
