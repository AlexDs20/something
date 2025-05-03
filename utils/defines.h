#if defined(__clang__)
#elif defined(__GNUC__)
#elif defined(_MSC_VER)
#elif defined(__INTEL_COMPILER)
#else
#endif

#define global_variable static
#define local_variable static
#define internal_function static
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
