#ifndef _ADS_STRING_H_
#define _ADS_STRING_H_

typedef struct {
    const char* buffer;
    size_t size;                    // bytes excluding \0
} StringView;

typedef struct {
    char* buffer;
    size_t size;                    // bytes excluding \0
    size_t capacity;                // bytes including \0
} String;

//==============================
// STRING
//==============================
String  string_init_empty(Arena* arena, size_t capacity);
String  string_init_fmt(Arena* arena, const char* fmt, ...);
String  string_init_sv(Arena* arena, StringView sv);
#define string_init_buffer(arena, buffer, len)  string_init_sv(arena, sv_from_buffer(buffer, len))
#define string_init_cstr(arena, cstr)           string_init_sv(arena, sv_from_cstr(cstr))

const char* string_as_cstr(const String* str);
int     string_grow_capacity(Arena* arena, String* str, size_t amount);

int     string_append_sv(Arena* arena, String* str, StringView append);
int     string_append_fmt(Arena* arena, String* str, const char* fmt, ...);
#define string_append_string(arena, str, str_p_append)      string_append_sv(arena, str, sv_from_string(*(str_p_append)))
#define string_append_cstr(arena, str, cstr_append)         string_append_sv(arena, str, sv_from_cstr(cstr_append))
#define string_append_buffer(arena, str, buf_append, len)   string_append_sv(arena, str, sv_from_buffer(buf_append, len))
#define string_append_char(arena, str, c)                   do { char t=c; string_append_sv(arena, str, sv_from_char(t)); } while (0)

int     string_prepend_sv(Arena* arena, String* str, StringView pre);
int     string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...);
#define string_prepend_string(arena, str, str_p_pre)        string_prepend_sv(arena, str, sv_from_string(*(str_p_pre)))
#define string_prepend_cstr(arena, str, cstr_pre)           string_prepend_sv(arena, str, sv_from_cstr(cstr_pre))
#define string_prepend_buffer(arena, str, buf_pre, len)     string_prepend_sv(arena, str, sv_from_buffer(buf_pre, len))
#define string_prepend_char(arena, str, c)                  do { char t=c; string_prepend_sv(arena, str, sv_from_char(t)); } while (0)

int    string_insert_buffer(Arena* arena, String* str, size_t pos, const char* buffer, size_t len);
int    string_insert_cstr(Arena* arena, String* str, size_t pos, const char* cstr);
int    string_insert_string(Arena* arena, String* str, size_t pos, const String* ins);
int    string_insert_sv(Arena* arena, String* str, size_t pos, StringView ins);
int    string_insert_char(Arena* arena, String* str, size_t pos, char c);
int    string_insert_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...);

/**
 * The overwrite functions allow for growth and thus include an arena because if we overwrite beyond the string capacity we need to maybe "realloc"
 * One could consider a version which does not allow for growth
 */
int    string_overwrite_buffer(Arena* arena, String* str, size_t pos, const char* buffer, size_t len);
int    string_overwrite_cstr(Arena* arena, String* str, size_t pos, const char* cstr);
int    string_overwrite_char(String* str, size_t pos, char c);
int    string_overwrite_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...);
// int    string_overwrite_string
// int    string_overwrite_sv

int    string_erase(String* str, size_t pos, size_t len);
// int    string_erase_and_insert(Arena* arena, String* str, size_t pos, size_t len, StringView sv);
int    string_clear(String* str);

String string_deep_copy(Arena* arena, const String* str);

void   string_debug_print(const String* string);
void   string_debug_print(String string);
void   string_print(const String* str);

//==============================
// STRING VIEW
//==============================
StringView sv_from_buffer(const char* buffer, size_t len);
StringView sv_from_string(String str);
StringView sv_from_cstr(const char* cstr);
#define     sv_from_char(c)     (StringView){&(c), 1}

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
