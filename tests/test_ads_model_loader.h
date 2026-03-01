#include "libs/ads_model_loader.h"

int test_model_read(void) {
    String filepath = string_init_cstr(arena, "assets/backpack/backpack.obj");
    Model* model = model_read(arena, sv_from_string(filepath));

    ASSERT_NOT_NULL(model);

    return 0;
}
