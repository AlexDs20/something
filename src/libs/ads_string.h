#ifndef _ADS_STRING_H_
#define _ADS_STRING_H_

typedef struct {
    const char* buffer;
    const size_t size;              // bytes excluding \0
} StringView;

typedef struct {
    char* buffer;
    size_t size;                    // bytes excluding \0
    size_t capacity;                // bytes including \0
} String;

//==============================
// STRING
//==============================
String string_init_empty(Arena* arena, size_t capacity);
String string_init_cstr(Arena* arena, const char* init);
String string_init_buffer(Arena* arena, const char* buffer, size_t len);
String string_init_concat(Arena* arena, const char* first, const char* second);
String string_init_fmt(Arena* arena, const char* fmt, ...);
String string_init_sv(Arena* arena, StringView sv);

int    string_append_buffer(Arena* arena, String* str, const char* buffer, size_t len);
int    string_append_cstr(Arena* arena, String* str, const char* post);
int    string_append_string(Arena* arena, String* str, const String* append);
int    string_append_sv(Arena* arena, String* str, StringView append);
int    string_append_char(Arena* arena, String* str, char c);
int    string_append_fmt(Arena* arena, String* str, const char* fmt, ...);

int    string_prepend_buffer(Arena* arena, String* str, const char* pre, size_t len);
int    string_prepend_cstr(Arena* arena, String* str, const char* pre);
int    string_prepend_string(Arena* arena, String* str, const String* pre);
int    string_prepend_sv(Arena* arena, String* str, StringView pre);
int    string_prepend_char(Arena* arena, String* str, char c);
int    string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...);

int    string_insert_buffer(Arena* arena, String* str, size_t pos, const char* buffer, size_t len);
int    string_insert_cstr(Arena* arena, String* str, size_t pos, const char* cstr);
int    string_insert_string(Arena* arena, String* str, size_t pos, const String* ins);
int    string_insert_sv(Arena* arena, String* str, size_t pos, StringView ins);
int    string_insert_char(Arena* arena, String* str, size_t pos, char c);
int    string_insert_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...);

// int    string_overwrite_cstr
// int    string_overwrite_string
// int    string_overwrite_sv
// int    string_overwrite_buffer
// int    string_overwrite_char
// int    string_overwrite_fmt

// int    string_erase

// String string_shallow_copy()
// String string_deep_copy()

// bool string_eq
// bool string_starts_with(const String* str, StringView prefix);
// bool string_ends_with(const String* str, StringView suffix);
// size_t string_find(const String* str, StringView needle);
// size_t string_rfind(const String* str, StringView needle);
// size_t string_find_from(const String* str, StringView needle, size_t start);
// size_t string_rfind_from(const String* str, StringView needle, size_t start);

// int string_clear();

void   string_debug_print(const String* string);
void   string_debug_print(String string);
void   string_print(const String* str);

//==============================
// STRING VIEW
//==============================
StringView sv_from_string(String str);
StringView sv_from_cstr(const char* cstr);
// StringView sv_from_buffer(const char* buffer, size_t len);
StringView sv_slice(StringView sv, size_t start, size_t len);
StringView sv_prefix(StringView sv, size_t len);
StringView sv_suffix(StringView sv, size_t len);

// bool sv_eq(StringView sv1, StringView sv2);
// bool sv_starts_with(StringView sv, StringView prefix);
// bool sv_ends_with(StringView sv, StringView suffix);
// size_t sv_find(StringView sv, StringView needle);
// size_t sv_rfind(StringView sv, StringView needle);
// size_t sv_find_from(StringView sv, StringView needle, size_t start);
// size_t sv_rfind_from(StringView sv, StringView needle, size_t start);

void sv_print(StringView sv);

void sv_debug_print(StringView sv);

#endif
