#include <stdio.h>
#include <time.h>
#include "tests/macros.h"
#include "tests/decode_jpeg.h"

typedef struct test_entry test_entry;
struct test_entry {
    char* name;
    int (*function)(void);
};

test_entry tests[] = {
    {"JPEG_progressive_rgb", test_read_progressive_rgb},
    {"JPEG_baseline_gray", test_read_baseline_gray},
    {"JPEG_baseline_rgb", test_read_baseline_rgb},
};

int main() {
    int failed = 0;

    for (int i=0; i<ARRAY_SIZE(tests); i++) {
        printf("%s started (%d/%d)", tests[i].name, i, ARRAY_SIZE(tests));
        clock_t start = clock();
        int r = tests[i].function();
        clock_t stop = clock();
        printf(" (%.4fs) ", (double)(stop - start) / CLOCKS_PER_SEC);
        if (r!=0) {
            failed++;
            printf("\tFailed");
        }
        printf("\n");
    }

    printf("Total failed tests: %d/%d\n", failed, ARRAY_SIZE(tests));

    return 0;
}
