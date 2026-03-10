#include "libs/ads_model_loader.h"

int test_model_read(void) {
    String filepath = string_init_cstr(arena, "assets/backpack/backpack.obj");
    Model* model = model_read(arena, sv_from_string(filepath));

    // ASSERT_NOT_NULL(model);

    filepath = string_init_cstr(arena, "../../../Downloads/006 - Charizard/BR_Charizard.obj");
    model = model_read(arena, sv_from_string(filepath));

    // ASSERT_NOT_NULL(model);

    filepath = string_init_cstr(arena, "../../../Downloads/sponza/sponza.obj");
    model = model_read(arena, sv_from_string(filepath));

    filepath = string_init_cstr(arena, "../../../Downloads/fireplace/fireplace_room.obj");
    model = model_read(arena, sv_from_string(filepath));

    return 0;
}
