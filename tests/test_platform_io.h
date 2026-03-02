#include "platform/io.h"
#include "libs/ads_string.h"

int test_read_complete_file(void) {
    const char* cstr =  "assets/backpack/backpack.obj";
    StringView filepath = sv_from_cstr(cstr);
    String file = read_complete_file(arena, filepath);
    return 0;
}
