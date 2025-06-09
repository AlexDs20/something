#ifndef _LIBSTRING_H
#define _LIBSTRING_H

#include "utils/types.h"
#include "memory/allocators.h"

string8 string_from_cstr(Arena* arena, const char* s);
char* string_to_cstr(Arena* arena, string8 s);
bool string_starts_with(string8 s, string8 pre);
bool string_starts_with(string8 s, const char* pre);
void string_move_forward(string8* s, u64 amount);
// string8 string_substring_up_to(string8 s, u64 to);
// string8 string_substring_from(string8 s, u64 from);
string8 string_copy_to_arena(Arena* arena, string8 string);
bool string_ends_with(string8 s, string8 pre);
// bool string_ends_with(string8 s, char* pre);
u64 string_find_first(string8 s, char c);
string8 string_substring(string8 s, u64 from, u64 to);
string8 string_dirname(string8 s);
string8 string_join_strings(Arena* arena, string8 a, string8 b);
string8 string_get_file_extension(string8 s);

bool operator==(string8 a, const char* c);
bool operator==(string8 a, string8 b);

void string_print(string8 s);


#endif // _LIBSTRING_H
