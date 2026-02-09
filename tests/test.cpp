#include <stdio.h>
#include <time.h>
#include "tests/macros.h"

#include "src/utils/defines.h"
#include "memory/allocators.h"
Arena* arena = arena_alloc_create_zero(1*GiB);

#include "tests/test_decode_jpeg.h"
#include "tests/test_ads_string.h"

typedef struct {
    char* name;
    int (*function)(void);
} test_entry;

test_entry tests[] = {
    // {"JPEG_baseline_rgb", test_read_baseline_rgb},
    // {"JPEG_baseline_gray", test_read_baseline_gray},
    // {"JPEG_baseline_gray_2", test_read_baseline_gray_2},

    {"test_string_init_empty", test_string_init_empty},
    {"test_string_init_fmt", test_string_init_fmt},
    {"test_string_init_sv", test_string_init_sv},
    {"test_string_init_cstr", test_string_init_cstr},
    {"test_string_init_buffer", test_string_init_buffer},

    {"test_string_grow_capacity", test_string_grow_capacity},

    {"test_string_append_fmt", test_string_append_fmt},
    {"test_string_append_sv", test_string_append_sv},
    {"test_string_append_string", test_string_append_string},
    {"test_string_append_cstr", test_string_append_cstr},
    {"test_string_append_buffer", test_string_append_buffer},
    {"test_string_append_char", test_string_append_char},

    {"test_string_prepend_fmt", test_string_prepend_fmt},
    // {"test_string_prepend_sv", test_string_prepend_sv},
    // {"test_string_prepend_string", test_string_prepend_string},
    // {"test_string_prepend_cstr", test_string_prepend_cstr},
    // {"test_string_prepend_buffer", test_string_prepend_buffer},
    // {"test_string_prepend_char", test_string_prepend_char},

    // {"test_string_insert_buffer", test_string_insert_buffer},
    // {"test_string_insert_fmt", test_string_insert_fmt},

    // {"test_string_overwrite_buffer", test_string_overwrite_buffer},
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
