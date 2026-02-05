#ifndef _ADS_STRING_H_
#define _ADS_STRING_H_

typedef struct {
    char* buffer;
    size_t len;
} StringView;

typedef struct {
    char* buffer;
    size_t size;
    size_t capacity;
} String;

String string_init_empty(Arena* arena, size_t capacity);
String string_init(Arena* arena, const char* init);
String string_init_concat(Arena* arena, const char* first, const char* second);
String string_init_fmt(Arena* arena, const char* fmt, ...);
String string_init_from_buffer(Arena* arena, const char* buffer, size_t len);
int    string_append(Arena* arena, String* str, const char* post);
int    string_append_len(Arena* arena, String* str, const char* buffer, size_t len);
int    string_append_char(Arena* arena, String* str, char c);
// int    string_append_fmt(Arena* arena, String* str, const char* fmt, ...);
int    string_prepend(Arena* arena, String* str, const char* pre);
int    string_prepend_len(Arena* arena, String* str, const char* pre, size_t len);
int    string_prepend_char(Arena* arena, String* str, char c);
// int    string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...);
void   string_debug_print(String* string);

String string_copy_slice(Arena* arena, String* str, size_t start, size_t len);

StringView string_slice(String str, size_t start, size_t len);
StringView string_to_sv(String str);
StringView sv_from_string(String str);
StringView sv_from_cstr(const char* cstr);

#endif
