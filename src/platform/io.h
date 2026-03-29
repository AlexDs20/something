#ifndef PLATFORM_IO_H
#define PLATFORM_IO_H

#include "libs/ads_string.h"
#include "memory/allocators.h"

#ifdef __cplusplus
extern "C" {
#endif

void read_n_bytes(char* filepath, unsigned int n, char* buffer);
String read_complete_file(Arena* arena, StringView filepath);

#ifdef __cplusplus
}
#endif

#endif // _PLATFORM_IO_H
