#ifndef _PLATFORM_IO_H
#define _PLATFORM_IO_H

#include "memory/allocators.h"
#include "utils/types.h"

string8 read_file(Arena* arena, string8 filepath);

#endif // _PLATFORM_IO_H
