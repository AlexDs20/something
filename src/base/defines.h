#ifndef ADS_DEFINES_H
#define ADS_DEFINES_H

#if defined(__clang__)
#elif defined(__GNUC__)
#elif defined(_MSC_VER)
#elif defined(__INTEL_COMPILER)
#else
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #include <stdalign.h>
    #define ALIGNAS(N) alignas(N)
#elif defined(_MSC_VER)
    #define ALIGNAS(N) __declspec(align(N))
#elif defined(__GNUC__) || defined(__clang__)
    #define ALIGNAS(N) __attribute__((aligned(N)))
#else
    #error "No alignment support for this compiler"
#endif

#define global static
#define local static
#define internal_function static
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define KiB (1024)               // KiB
#define MiB (1024*1024)          // MiB
#define GiB (1024*1024*1024)     // GiB

#if defined(ADS_DEBUG)
#define ASSERT(x)       do { (x) ? 0 : *(volatile char*) 0 = __LINE__; } while (0)
#else
#define ASSERT(x)       do { } while (0)
#endif

#if defined(ADS_DEBUG)
#define PANIC           do { *(volatile char*) 0 = __LINE__; } while (0)
#else
#define PANIC
#endif

#define NOT_IMPLEMENTED(x)      do { printf( "\n" #x " not implemented." ); } while(0)

#endif // ADS_DEFINES_H
