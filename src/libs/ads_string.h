#ifndef _ADS_STRING_H_
#define _ADS_STRING_H_

typedef struct {
    char* buffer;
    size_t size;
} StringView;

typedef struct {
    char* buffer;
    size_t size;
    size_t capacity;
} String;

//==============================
// STRING
//==============================
String string_init_empty(Arena* arena, size_t capacity);
String string_init_cstr(Arena* arena, const char* init);
String string_init_concat(Arena* arena, const char* first, const char* second);
String string_init_fmt(Arena* arena, const char* fmt, ...);
String string_init_buffer(Arena* arena, const char* buffer, size_t len);
// String string_init_sv(Arena* arena, StringView sv);

int    string_append_cstr(Arena* arena, String* str, const char* post);
// int    string_append_string(Arena* arena, String* str, String append);
// int    string_append_sv(Arena* arena, String* str, StringView append);
int    string_append_buffer(Arena* arena, String* str, const char* buffer, size_t len);
int    string_append_char(Arena* arena, String* str, char c);
// int    string_append_fmt(Arena* arena, String* str, const char* fmt, ...);

int    string_prepend_cstr(Arena* arena, String* str, const char* pre);
// int    string_prepend_string(Arena* arena, String* str, String pre);
// int    string_prepend_sv(Arena* arena, String* str, StringView pre);
int    string_prepend_buffer(Arena* arena, String* str, const char* pre, size_t len);
int    string_prepend_char(Arena* arena, String* str, char c);
// int    string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...);

void   string_debug_print(String* string);

//==============================
// STRING VIEW
//==============================
StringView string_slice(String str, size_t start, size_t len);
StringView string_to_sv(String str);
StringView sv_from_string(String str);
StringView sv_from_cstr(const char* cstr);
StringView sv_from_buffer(const char* buffer, size_t len);
StringView sv_slice(StringView sv);

#endif
