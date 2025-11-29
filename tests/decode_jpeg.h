#include "libs/ads_images.h"
#include "memory/allocators.h"
#include "utils/types.h"
#include "utils/defines.h"

Arena* arena = arena_alloc_create_zero(1*GiB);

int test_read_baseline_rgb() {
    string8 filename = string_from_cstr(arena, "assets/backpack/diffuse.jpg");
    read_image_file(arena, filename);
    return 0;
}

int test_read_baseline_gray() {
    string8 filename = string_from_cstr(arena, "assets/backpack/specular.jpg");
    read_image_file(arena, filename);
    return 0;
}
