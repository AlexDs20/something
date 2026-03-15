#ifndef _PLATFORM_IO_H
#define _PLATFORM_IO_H

#include "memory/allocators.h"
#include "utils/types.h"
#include "libs/ads_string.h"

void read_n_bytes(char* filepath, unsigned int n, char* buffer);
String read_complete_file(Arena* arena, StringView filepath);

#endif // _PLATFORM_IO_H
