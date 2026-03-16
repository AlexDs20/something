#ifndef _PLATFORM_IO_H
#define _PLATFORM_IO_H

#include "libs/ads_string.h"
struct Arena;

void read_n_bytes(char* filepath, unsigned int n, char* buffer);
String read_complete_file(Arena* arena, StringView filepath);

#endif // _PLATFORM_IO_H
