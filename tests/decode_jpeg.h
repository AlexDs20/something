#include "libs/ads_images.h"
#include "memory/allocators.h"
#include "utils/types.h"
#include "utils/defines.h"

Arena* arena = arena_alloc_create_zero(1*GiB);

int test_read_baseline_rgb() {
    string8 filename = string_from_cstr(arena, "assets/backpack/diffuse.jpg");
    read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}

int test_read_baseline_gray() {
    string8 filename = string_from_cstr(arena, "assets/backpack/specular.jpg");
    Image image = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}

int test_read_progressing_failing_other3() {
    string8 filename = string_from_cstr(arena, "assets/backpack/other3.jpg");
    Image image = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}

int test_read_baseline_other3() {
    string8 filename = string_from_cstr(arena, "assets/backpack/other3_baseline.jpg");
    Image image = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}

int test_read_progressive_other3() {
    string8 filename = string_from_cstr(arena, "assets/backpack/other3_prog.jpg");
    Image image = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}

int test_read_progressive_test5() {
    string8 filename = string_from_cstr(arena, "assets/backpack/test5.jpg");
    Image image = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}

int test_read_progressive_test11() {
    string8 filename = string_from_cstr(arena, "assets/backpack/test11.jpg");
    Image image = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}

int test_read_progressive_test12() {
    string8 filename = string_from_cstr(arena, "assets/backpack/test12.jpg");
    Image image = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}

int test_read_progressive_rgb() {
    string8 filename = string_from_cstr(arena, "assets/backpack/diffuse_restart_marker_progressive.jpg");
    Image image = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}
