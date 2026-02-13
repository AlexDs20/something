#ifndef _ADS_STRING_H_
#define _ADS_STRING_H_
#include <stdarg.h>     // for variadic number of args.

// TODO?
typedef enum {
    STRING_SUCCESS = 0,
    // STRING_NULL,
    // STRING_ARENA_FAIL,
    // STRING_OUT_OF_MEMORY,
    // STRING_OUT_OF_BOUNDS,
    STRING_FAIL,
} StringError;

typedef struct {
    const char* buffer;
    size_t size;                    // bytes excluding \0
} StringView;

typedef struct {
    char* buffer;
    size_t size;                    // Length of string excluding \0 (like strlen)
    size_t capacity;                // Total allocated size including \0
} String;

//==============================
// STRING
//==============================
String  string_init_empty(Arena* arena, size_t capacity);
String  string_init_fmt(Arena* arena, const char* fmt, ...);
String  string_init_vfmt(Arena* arena, const char* fmt, va_list args);
String  string_init_sv(Arena* arena, StringView sv);
#define string_init_buffer(arena, buffer, len)  string_init_sv(arena, sv_from_buffer(buffer, len))
#define string_init_cstr(arena, cstr)           string_init_sv(arena, sv_from_cstr(cstr))

const char* string_as_cstr(const String* str);
#define string_as_sv(str_p)    (StringView){.buffer=(str_p)->buffer, .size=(str_p)->size}

int     string_append_fmt(Arena* arena, String* str, const char* fmt, ...);
int     string_append_vfmt(Arena* arena, String* str, const char* fmt, va_list args);
int     string_append_sv(Arena* arena, String* str, StringView append);
#define string_append_string(arena, str, str_p_append)      string_append_sv(arena, str, sv_from_string(*(str_p_append)))
#define string_append_cstr(arena, str, cstr_append)         string_append_sv(arena, str, sv_from_cstr(cstr_append))
#define string_append_buffer(arena, str, buf_append, len)   string_append_sv(arena, str, sv_from_buffer(buf_append, len))
#define string_append_char(arena, str, c)                   do { char _c=(c); string_append_sv(arena, str, sv_from_buffer(&(_c), 1)); } while (0)

int     string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...);
int     string_prepend_vfmt(Arena* arena, String* str, const char* fmt, va_list args);
int     string_prepend_sv(Arena* arena, String* str, StringView pre);
#define string_prepend_string(arena, str, str_p_pre)        string_prepend_sv(arena, str, sv_from_string(*(str_p_pre)))
#define string_prepend_cstr(arena, str, cstr_pre)           string_prepend_sv(arena, str, sv_from_cstr(cstr_pre))
#define string_prepend_buffer(arena, str, buf_pre, len)     string_prepend_sv(arena, str, sv_from_buffer(buf_pre, len))
#define string_prepend_char(arena, str, c)                  do { char _c=(c); string_prepend_sv(arena, str, sv_from_buffer(&(_c), 1)); } while (0)

int     string_insert_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...);
int     string_insert_vfmt(Arena* arena, String* str, size_t pos, const char* fmt, va_list args);
int     string_insert_sv(Arena* arena, String* str, size_t pos, StringView ins);
#define string_insert_string(arena, str, pos, str_p_ins)    string_insert_sv(arena, str, pos, sv_from_string(*(str_p_ins)))
#define string_insert_cstr(arena, str, pos, cstr)           string_insert_sv(arena, str, pos, sv_from_cstr(cstr))
#define string_insert_buffer(arena, str, pos, buffer, len)  string_insert_sv(arena, str, pos, sv_from_buffer(buffer, len))
#define string_insert_char(arena, str, c)                   do { char _c=(c); string_insert_sv(arena, str, pos, sv_from_buffer(&(_c), 1)); } while (0)

int     string_overwrite_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...);
int     string_overwrite_vfmt(Arena* arena, String* str, size_t pos, const char* fmt, va_list args);
int     string_overwrite_sv(Arena* arena, String* str, size_t pos, StringView sv);
#define string_overwrite_buffer(arena, str, pos, buffer, len)      string_overwrite_sv(arena, str, pos, sv_from_buffer(buffer, len))
#define string_overwrite_cstr(arena, str, pos, cstr)               string_overwrite_sv(arena, str, pos, sv_from_cstr(cstr))
#define string_overwrite_string(arena, str, pos, str_p_over)       string_overwrite_sv(arena, str, pos, sv_from_string(*(str_p_over)))
#define string_overwrite_char(arena, str, pos, c)                  do { char _c=(c); string_overwrite_sv(arena, str, pos, sv_from_buffer(&(_c), 1)); } while (0)

int     string_erase_and_insert_fmt(Arena* arena, String* str, size_t pos, size_t len, const char* fmt, ...);
int     string_erase_and_insert_vfmt(Arena* arena, String* str, size_t pos, size_t len, const char* fmt, va_list args);
int     string_erase_and_insert_sv(Arena* arena, String* str, size_t pos, size_t len, StringView sv);

int    string_clear(String* str);
int    string_erase(String* str, size_t pos, size_t len);
String string_deep_copy(Arena* arena, const String* str);

void   string_debug_print(const String* string);
void   string_debug_print(String string);
void   string_print(const String* str);

//==============================
// STRING VIEW
//==============================
StringView  sv_from_buffer(const char* buffer, size_t len);
StringView  sv_from_cstr(const char* cstr);
#define     sv_from_string(str)     (StringView){.buffer=(str).buffer, .size=(str).size}

StringView  sv_slice_sv(StringView sv, size_t start, size_t len);
#define     sv_slice_string(str, start, len)        sv_slice_sv(sv_from_string(str), start, len)
StringView sv_truncate_front(StringView sv, size_t len);
StringView sv_truncate_back(StringView sv, size_t len);
StringView sv_trim_front(StringView sv);         // removes ' ', '\t', '\n', '\r', '\v'. '\f'
StringView sv_trim_back(StringView sv);
// StringView sv_split_delim(StringView* sv, char delim)
// StringView sv_file_extension(StringView sv);
// StringView sv_file_name(StringView sv);
// StringView sv_directory_name(StringView sv);

bool sv_equal(StringView sv1, StringView sv2);
int  sv_compare(const StringView* sv1, const StringView* sv2);         // For usage with e.g. qsort or binary search
bool sv_starts_with(StringView sv, StringView prefix);
bool sv_ends_with(StringView sv, StringView suffix);
size_t sv_find(StringView haystack, StringView needle);
size_t sv_rfind(StringView haystack, StringView needle);

void sv_print(StringView sv);
void sv_debug_print(StringView sv);

#endif  // _ADS_STRING_H_
