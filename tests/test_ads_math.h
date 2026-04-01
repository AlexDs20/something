#include "libs/ads_math.h"

int test_ads_math(void) {
    ASSERT_EQ(f32_sign(-4.0f), -1.0f);
    ASSERT_EQ(f32_sign( 4.0f),  1.0f);
    ASSERT_EQ(f32_sign( 0.0f),  0.0f);

    return 0;
}
