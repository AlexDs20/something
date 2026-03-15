#include "libs/ads_images.h"
#include "memory/allocators.h"
#include "utils/types.h"
#include "utils/defines.h"

int test_read_baseline_rgb() {
    StringView filename = sv_from_cstr("assets/backpack/diffuse.jpg");
    Image img = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}

int test_read_baseline_gray() {
    StringView filename = sv_from_cstr("assets/backpack/specular.jpg");
    Image img = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}

int test_read_baseline_gray_2() {
    StringView filename = sv_from_cstr("assets/backpack/roughness.jpg");
    Image img = read_image_file(arena, filename);
    // ASSERT_NOT_NULL(image.buffer);
    return 0;
}
