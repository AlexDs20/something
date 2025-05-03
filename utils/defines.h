#ifndef _DEFINES_H
#define _DEFINES_H

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

#define kB (1024)               // KiB
#define MB (1024*1024)          // MiB
#define GB (1024*1024*1024)     // GiB

#define int8  char
#define int16 short
#define int32 int
#define int64 long

#define uint8  unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define uint64 unsigned long

#define float32 float
#define float64 double

#endif // _DEFINES_H
