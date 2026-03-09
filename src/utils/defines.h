#ifndef _DEFINES_H
#define _DEFINES_H

#if defined(__clang__)
#elif defined(__GNUC__)
#elif defined(_MSC_VER)
#elif defined(__INTEL_COMPILER)
#else
#endif

#define global static
#define local static
#define internal_function static
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define KiB (1024)               // KiB
#define MiB (1024*1024)          // MiB
#define GiB (1024*1024*1024)     // GiB

#define EPS (1e-6)

#if defined(__DEBUG__)
#define ASSERT(x)       do { (x) ? 0 : *(volatile char*) 0 = __LINE__; } while (0)
#else
#define ASSERT(x)       do { } while (0)
#endif

#endif // _DEFINES_H
