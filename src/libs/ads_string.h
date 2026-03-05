#ifndef _ADS_STRING_H_
#define _ADS_STRING_H_
#include <stdarg.h>     // for variadic number of args.
#include <stddef.h>     // size_t
#include <stdbool.h>
#include <stdint.h>     // UINT32_MAX, ...

// TODO:
// Small String Optimization
// UTF8
// Format logic repeated
// Allocator abstraction layer?

// TODO?
typedef enum {
    STRING_SUCCESS = 0,
    // STRING_OUT_OF_MEMORY,
    // STRING_BAD_INPUT,
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
// STRING VIEW
//==============================
StringView  sv_from_sv(StringView sv);
StringView  sv_from_buffer(const char* buffer, size_t len);
StringView  sv_from_cstr(const char* cstr);
StringView  sv_from_string(String str);

StringView  sv_slice_sv(StringView sv, size_t start, size_t len);
#define     sv_slice_string(str, start, len)        sv_slice_sv(sv_from_string((str)), (start), (len))
StringView  sv_truncate_front(StringView sv, size_t len);
StringView  sv_truncate_back(StringView sv, size_t len);
StringView  sv_trim_front(StringView sv);         // removes ' ', '\t', '\n', '\r', '\v'. '\f'
StringView  sv_trim_back(StringView sv);
// TODO
// StringView sv_trim_front_by_chars(StringView sv, StringView chars);
// StringView sv_trim_back_by_chars(StringView sv, StringView chars);
static inline bool      sv_is_empty(StringView sv) { return sv.size == 0; }

StringView sv_chop_by_delim_sv(StringView* sv, StringView delim);
#define    sv_chop_by_delim_cstr(sv_ptr, cstr)           sv_chop_by_delim_sv((sv_ptr), sv_from_cstr((cstr)))
#define    sv_chop_by_delim_string(sv_ptr, str)          sv_chop_by_delim_sv((sv_ptr), sv_from_string((str)))
#define    sv_chop_by_delim_buffer(sv_ptr, buf, len)     sv_chop_by_delim_sv((sv_ptr), sv_from_buffer((buf), (len)))
static inline StringView sv_chop_by_delim_char(StringView* sv, char c) { return sv_chop_by_delim_sv(sv, sv_from_buffer(&c, 1)); }

StringView sv_chop_by(StringView* sv, size_t pos);

StringView sv_file_extension(StringView sv);
StringView sv_file_name(StringView sv);
StringView sv_directory_name(StringView sv);

bool sv_equal(StringView sv1, StringView sv2);
int  sv_compare(const StringView* sv1, const StringView* sv2);         // For usage with e.g. qsort or binary search
bool sv_starts_with(StringView sv, StringView prefix);
bool sv_starts_with_char(StringView sv, char c);
#define sv_starts_with_cstr(sv, cstr)       sv_starts_with((sv), sv_from_cstr((cstr)))
bool sv_ends_with(StringView sv, StringView suffix);
size_t sv_find(StringView haystack, StringView needle);
size_t sv_rfind(StringView haystack, StringView needle);
// TODO
// size_t sv_find_any_of(StringView sv, StringView chars);
// size_t sv_rfind_any_of(StringView sv, StringView chars);
// size_t sv_count_of(StringView sv, StringView pattern);

const char* sv_as_cstr(Arena* arena, StringView sv);

//int sv_parse_u8(StringView* sv, uint8_t* out);
//int sv_parse_s8(StringView* sv, int8_t* out);
//int sv_parse_u16(StringView* sv, uint16_t* out);
//int sv_parse_s16(StringView* sv, int16_t* out);
int sv_parse_u32(StringView* sv, uint32_t* out);
int sv_parse_s32(StringView* sv, int32_t* out);
//int sv_parse_u64(StringView* sv, uint64_t* out);
//int sv_parse_s64(StringView* sv, int64_t* out);
int sv_parse_f32(StringView* sv, float* out);
//int sv_parse_f64(StringView* sv, double* out);

void sv_print(StringView sv);
void sv_debug_print(StringView sv);

//==============================
// STRING
//==============================
String  string_init_empty(Arena* arena, size_t capacity);
String  string_init_fmt(Arena* arena, const char* fmt, ...);
String  string_init_vfmt(Arena* arena, const char* fmt, va_list args);
String  string_init_sv(Arena* arena, StringView sv);
#define string_init_buffer(arena, buffer, len)  string_init_sv((arena), sv_from_buffer((buffer), (len)))
#define string_init_cstr(arena, cstr)           string_init_sv((arena), sv_from_cstr((cstr)))

const char* string_as_cstr(const String* str);
#define string_as_sv(str_p)    (StringView){.buffer=(str_p)->buffer, .size=(str_p)->size}

int     string_append_fmt(Arena* arena, String* str, const char* fmt, ...);
int     string_append_vfmt(Arena* arena, String* str, const char* fmt, va_list args);
int     string_append_sv(Arena* arena, String* str, StringView append);
#define string_append_string(arena, str, str_p_append)      string_append_sv((arena), (str), sv_from_string(*(str_p_append)))
#define string_append_cstr(arena, str, cstr_append)         string_append_sv((arena), (str), sv_from_cstr((cstr_append)))
#define string_append_buffer(arena, str, buf_append, len)   string_append_sv((arena), (str), sv_from_buffer((buf_append), (len)))
static inline int string_append_char(Arena* arena, String* str, char c) { return string_append_sv(arena, str, sv_from_buffer(&c, 1)); }

int     string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...);
int     string_prepend_vfmt(Arena* arena, String* str, const char* fmt, va_list args);
int     string_prepend_sv(Arena* arena, String* str, StringView pre);
#define string_prepend_string(arena, str, str_p_pre)        string_prepend_sv((arena), (str), sv_from_string(*(str_p_pre)))
#define string_prepend_cstr(arena, str, cstr_pre)           string_prepend_sv((arena), (str), sv_from_cstr((cstr_pre)))
#define string_prepend_buffer(arena, str, buf_pre, len)     string_prepend_sv((arena), (str), sv_from_buffer((buf_pre), (len)))
static inline int string_prepend_char(Arena* arena, String* str, char c) { return string_prepend_sv(arena, str, sv_from_buffer(&c, 1)); }

int     string_insert_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...);
int     string_insert_vfmt(Arena* arena, String* str, size_t pos, const char* fmt, va_list args);
int     string_insert_sv(Arena* arena, String* str, size_t pos, StringView ins);
#define string_insert_string(arena, str, pos, str_p_ins)    string_insert_sv((arena), (str), (pos), sv_from_string(*(str_p_ins)))
#define string_insert_cstr(arena, str, pos, cstr)           string_insert_sv((arena), (str), (pos), sv_from_cstr((cstr)))
#define string_insert_buffer(arena, str, pos, buffer, len)  string_insert_sv((arena), (str), (pos), sv_from_buffer((buffer), (len)))
static inline int string_insert_char(Arena* arena, String* str, size_t pos, char c) { return string_insert_sv(arena, str, pos, sv_from_buffer(&c, 1)); }

int     string_overwrite_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...);
int     string_overwrite_vfmt(Arena* arena, String* str, size_t pos, const char* fmt, va_list args);
int     string_overwrite_sv(Arena* arena, String* str, size_t pos, StringView sv);
#define string_overwrite_buffer(arena, str, pos, buffer, len)      string_overwrite_sv((arena), (str), (pos), sv_from_buffer((buffer), (len)))
#define string_overwrite_cstr(arena, str, pos, cstr)               string_overwrite_sv((arena), (str), (pos), sv_from_cstr((cstr)))
#define string_overwrite_string(arena, str, pos, str_p_over)       string_overwrite_sv((arena), (str), (pos), sv_from_string(*(str_p_over)))
static inline int string_overwrite_char(Arena* arena, String* str, size_t pos, char c) { return string_overwrite_sv(arena, str, pos, sv_from_buffer(&c, 1)); }

int     string_erase_and_insert_fmt(Arena* arena, String* str, size_t pos, size_t len, const char* fmt, ...);
int     string_erase_and_insert_vfmt(Arena* arena, String* str, size_t pos, size_t len, const char* fmt, va_list args);
int     string_erase_and_insert_sv(Arena* arena, String* str, size_t pos, size_t len, StringView sv);
#define string_erase_and_insert_buffer(arena, str, pos, rm_len, buffer, ins_len)    string_erase_and_insert_sv((arena), (str), (pos), (rm_len), sv_from_buffer((buffer), (ins_len)))
#define string_erase_and_insert_cstr(arena, str, pos, rm_len, cstr)                 string_erase_and_insert_sv((arena), (str), (pos), (rm_len), sv_from_cstr((cstr)))
#define string_erase_and_insert_string(arena, str, pos, rm_len, str_p_over)         string_erase_and_insert_sv((arena), (str), (pos), (rm_len), sv_from_string(*(str_p_over)))
static inline int string_erase_and_insert_char(Arena* arena, String* str, size_t pos, size_t rm_len, char c) { return string_erase_and_insert_sv(arena, str, pos, rm_len, sv_from_buffer(&c, 1)); }

int string_replace_first(Arena* arena, String* string, StringView target, StringView replacement);
int string_replace_last(Arena* arena, String* string, StringView target, StringView replacement);
int string_replace_all(Arena* arena, String* str, StringView target, StringView replacement);

int     string_clear(String* str);
int     string_erase(String* str, size_t pos, size_t len);
String  string_deep_copy(Arena* arena, String str);
int     string_reserve(Arena* arena, String* str, size_t new_capacity);
#define string_increase_capacity(arena, str, amount)  string_reserve((arena), (str), (str) ? (str)->size + (amount) : 0)
int     string_shrink_to_fit(Arena* arena, String* str);

void    string_print(const String* str);
void    string_debug_print(const String* string);

#endif  // _ADS_STRING_H_
