#include <string.h>
#include <stdio.h>

#include "memory/allocators.h"
#include "libs/ads_string.h"

static size_t _get_new_capacity(size_t old_capacity, size_t new_size) {
    size_t new_cap;
    if (old_capacity > SIZE_MAX / 2) {
        if (new_size == SIZE_MAX) {
            return SIZE_MAX;
        }
        return new_size + 1;
    }
    new_cap = 2*old_capacity > new_size+1 ? 2*old_capacity : new_size+1;
    return new_cap > 16 ? new_cap : 16;
}

static int _compare(const void* v1, const void* v2, size_t len) {
#if 1
    return memcmp(v1, v2, len);
#else
    const unsigned char* s1 = (const unsigned char*)v1;
    const unsigned char* s2 = (const unsigned char*)v2;
    while (len-- > 0) {
        if (*s1 != *s2) {
            return *s1 < *s2 ? -1 : 1;
        }
        s1++;
        s2++;
    }
    return 0;
#endif
}

static int _resize_and_shift_memory_logic(Arena* arena, String* str, size_t pos, size_t rm_len, size_t insert_len) {
    if (str == NULL || str->buffer == NULL) {
        return -1;
    }
    if (pos > str->size || pos + rm_len > str->size) {
        return -1;
    }
    if (insert_len > rm_len && SIZE_MAX - str->size < insert_len - rm_len) {
        return -1;
    }
    size_t new_size = str->size - rm_len + insert_len;

    // If we grow beyond the current capacity => need more space
    if (new_size + 1 > str->capacity) {
        size_t new_capacity = _get_new_capacity(str->capacity, new_size);
        char* arena_top = (char*)arena_alloc_used_location(arena);

        if (arena_top == str->buffer + str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) return -1;
            if (str->size+1-(pos+rm_len) > 0) {
                memmove(str->buffer+pos+insert_len,
                        str->buffer+pos+rm_len,
                        str->size+1-(pos+rm_len));
            }
        }
        else {
            char* t = (char*)arena_alloc_push(arena, new_capacity);
            if (t == NULL) return -1;

            if (pos > 0) {
                memcpy(t, str->buffer, pos);
            }

            if (str->size+1-(pos+rm_len) > 0) {
                memcpy(t+pos+insert_len,
                       str->buffer+pos+rm_len,
                       str->size+1-(pos+rm_len));
            }
            str->buffer = t;
        }
        str->capacity = new_capacity;
    }
    else {
        if (str->size+1-(pos+rm_len) > 0) {
            memmove(str->buffer+pos+insert_len,
                    str->buffer+pos+rm_len,
                    str->size+1-(pos+rm_len));
        }
    }
    str->size = new_size;
    str->buffer[str->size] = '\0';
    return 0;
}

//==============================
// STRING
//==============================
String string_init_empty(Arena* arena, size_t capacity) {
    String str;
    if (capacity == 0) {
        capacity = 1;
    }

    str.buffer = (char*) arena_alloc_push(arena, capacity);

    if (str.buffer == NULL) {
        return (String){0};
    }

    str.capacity = capacity;
    str.size = 0;
    str.buffer[0] = '\0';
    return str;
}

String string_init_fmt(Arena* arena, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    String out = string_init_vfmt(arena, fmt, ap);
    va_end(ap);
    return out;
}

String string_init_vfmt(Arena* arena, const char* fmt, va_list args) {
    va_list args2;
    String str;
    if (fmt == NULL) {
        return (String){0};
    }

    char fmt_buffer[128];

    va_copy(args2, args);
    int n = vsnprintf(fmt_buffer, sizeof(fmt_buffer), fmt, args2);
    va_end(args2);
    if (n < 0) {
        return (String){0};
    }
    size_t len = (size_t)n;
    size_t capacity = _get_new_capacity(0, len);

    if (len < sizeof(fmt_buffer)) {
        char* buf = (char*) arena_alloc_push(arena, capacity);
        if (buf == NULL) {
            return (String){0};
        }
        memcpy(buf, fmt_buffer, len+1);
        str.buffer = buf;
        str.capacity = capacity;
        str.size = len;
        return str;
    }


    str.buffer = (char*) arena_alloc_push(arena, capacity);
    if (str.buffer == NULL) {
        return (String){0};
    }

    va_copy(args2, args);
    int m = vsnprintf(str.buffer, len+1, fmt, args2);          // Write len+1 elements including '\0'
    va_end(args2);
    if (m < 0 || (size_t) m != len) {
        return (String){0};
    }

    str.capacity = capacity;
    str.size = len;
    return str;
}

String string_init_sv(Arena* arena, StringView sv) {
    String str;

    if (sv.buffer == NULL) {
        return (String){0};
    }

    str.capacity = _get_new_capacity(0, sv.size);
    str.buffer = (char*) arena_alloc_push(arena, str.capacity);

    if (str.buffer == NULL) {
        return (String){0};
    }

    memcpy(str.buffer, sv.buffer, sv.size);
    str.size = sv.size;
    str.buffer[str.size] = '\0';
    return str;
}

const char* string_as_cstr(const String* str) {
    if (str == NULL || str->buffer == NULL) {
        return "";
    }
    return str->buffer;
}

static int append_memory_logic(Arena* arena, String* str, size_t append_len) {
    return _resize_and_shift_memory_logic(arena, str, str->size, 0, append_len);
}

int string_append_vfmt(Arena* arena, String* str, const char* fmt, va_list args){
    va_list args2;
    if (str == NULL || str->buffer == NULL || fmt == NULL) {
        return -1;
    }

    //==================================================
    // Source: https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Variable-Number-of-Arguments.html
    char fmt_buffer[128];
    va_copy(args2, args);
    int n = vsnprintf(fmt_buffer, sizeof(fmt_buffer), fmt, args2);
    va_end(args2);
    if (n < 0) {
        return -1;
    }
    size_t len = (size_t)n;
    size_t old_size = str->size;

    int r = append_memory_logic(arena, str, len);
    if (r != 0) {
        return r;
    }

    if (len < sizeof(fmt_buffer)) {
        memcpy(str->buffer+old_size, fmt_buffer, len);
    }
    else {
        va_copy(args2, args);
        int m = vsnprintf(str->buffer+old_size, len+1, fmt, args2);
        va_end(args2);
        if (m < 0 || (size_t) m != len) {
            return -1;
        }
    }

    return 0;
}

int string_append_fmt(Arena* arena, String* str, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = string_append_vfmt(arena, str, fmt, ap);
    va_end(ap);
    return r;
}

int string_append_sv(Arena* arena, String* str, StringView append) {
    if (str == NULL || str->buffer == NULL || append.buffer == NULL) {
        return -1;
    }

    size_t old_size = str->size;
    int r = append_memory_logic(arena, str, append.size);
    if (r != 0) {
        return r;
    }

    memcpy(str->buffer+old_size, append.buffer, append.size);
    return 0;
}

static int prepend_memory_logic(Arena* arena, String* str, size_t prepend_len) {
    return _resize_and_shift_memory_logic(arena, str, 0, 0, prepend_len);
}

int string_prepend_vfmt(Arena* arena, String* str, const char* fmt, va_list args) {
    va_list args2;

    if (str == NULL || str->buffer == NULL || fmt == NULL) {
        return -1;
    }
    char fmt_buffer[128];
    va_copy(args2, args);
    int n = vsnprintf(fmt_buffer, sizeof(fmt_buffer), fmt, args2);
    va_end(args2);
    if (n<0) {
        return -1;
    }
    size_t len = (size_t) n;

    int r = prepend_memory_logic(arena, str, len);
    if (r != 0) {
        return r;
    }

    if (len < sizeof(fmt_buffer)) {
        memcpy(str->buffer, fmt_buffer, len);
    }
    else {
        const char save_char = str->buffer[len];
        va_copy(args2, args);
        int m = vsnprintf(str->buffer, len+1, fmt, args2);
        va_end(args2);
        // Note: This breaks immutability on error
        if (m < 0 || (size_t) m != len) {
            return -1;
        }
        str->buffer[len] = save_char;
    }
    return 0;
}

int string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = string_prepend_vfmt(arena, str, fmt, ap);
    va_end(ap);
    return r;
}

int string_prepend_sv(Arena* arena, String* str, StringView pre) {
    size_t new_size;
    char* arena_top;
    char* tmp;

    if ((str == NULL) || (str->buffer == NULL) || (pre.buffer == NULL)) {
        return -1;
    }

    LocalArena* local_arena = NULL;
    const char* pre_buffer = pre.buffer;

    // If the StringView is a part the string we prepend
    // Copy to a side buffer
    if ((uintptr_t)str->buffer < (uintptr_t)pre.buffer + pre.size &&
        (uintptr_t)pre.buffer < (uintptr_t)str->buffer + str->size) {

        local_arena = local_arena_alloc_create();
        if (local_arena == NULL) {
            return -1;
        }

        char* t = (char*)arena_alloc_push_struct(local_arena->arena, (void*)pre.buffer, pre.size);
        if (t == NULL) {
            local_arena_alloc_reset(local_arena);
            return -1;
        }
        pre_buffer = t;
    }

    int r = prepend_memory_logic(arena, str, pre.size);
    if (r != 0) {
        if (local_arena != NULL) {
            local_arena_alloc_reset(local_arena);
        }
        return r;
    }

    memcpy(str->buffer, pre_buffer, pre.size);
    if (local_arena != NULL) {
        local_arena_alloc_reset(local_arena);
    }
    return 0;
}

static int insert_memory_logic(Arena* arena, String* str, size_t pos, size_t ins_len) {
    return _resize_and_shift_memory_logic(arena, str, pos, 0, ins_len);
}

int string_insert_vfmt(Arena* arena, String* str, size_t pos, const char* fmt, va_list args) {
    va_list args2;
    if (str == NULL || str->buffer == NULL || fmt == NULL || pos > str->size) {
        return -1;
    }

    va_copy(args2, args);
    if (pos == 0) {
        int r = string_prepend_vfmt(arena, str, fmt, args2);
        va_end(args2);
        return r;
    } else if (pos == str->size) {
        int r = string_append_vfmt(arena, str, fmt, args2);
        va_end(args2);
        return r;
    }

    char fmt_buffer[128];
    va_copy(args2, args);
    int n = vsnprintf(fmt_buffer, sizeof(fmt_buffer), fmt, args2);
    va_end(args2);
    if (n<0) {
        return -1;
    }
    size_t len = (size_t) n;

    int r = insert_memory_logic(arena, str, pos, len);
    if (r != 0) {
        return r;
    }

    // Save first char of what is after the insert because vsnprintf always puts a \0 ...
    if (len < sizeof(fmt_buffer)) {
        memcpy(str->buffer+pos, fmt_buffer, len);
    }
    else {
        char save_char = str->buffer[pos+len];
        va_copy(args2, args);
        int m = vsnprintf(str->buffer+pos, len+1, fmt, args2);
        va_end(args2);
        // Note: the string has been mutated and if error => string is dirty
        if (m < 0 || (size_t) m != len) {
            return -1;
        }
        str->buffer[pos+len] = save_char;
    }
    return 0;
}

int string_insert_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int r = string_insert_vfmt(arena, str, pos, fmt, ap);
    va_end(ap);
    return r;
}

int string_insert_sv(Arena* arena, String* str, size_t pos, StringView ins) {
    if ((str == NULL) || (str->buffer == NULL) || (pos > str->size) || (ins.buffer == NULL)) {
        return -1;
    }

    if (pos == 0) {
        return string_prepend_sv(arena, str, ins);
    }
    else if (pos == str->size) {
        return string_append_sv(arena, str, ins);
    }

    LocalArena* local_arena = NULL;
    const char* ins_buffer = ins.buffer;

    // If the StringView is partly after where we insert
    // Copy to a side buffer
    if ((uintptr_t)str->buffer < (uintptr_t)ins.buffer + ins.size &&
        (uintptr_t)ins.buffer < (uintptr_t)str->buffer + str->size) {

        local_arena = local_arena_alloc_create();
        if (local_arena == NULL) {
            return -1;
        }

        // TODO: Optimization ? Only copy the part intersecting what moves
        char* t = (char*)arena_alloc_push_struct(local_arena->arena, (void*)ins.buffer, ins.size);
        if (t == NULL) {
            local_arena_alloc_reset(local_arena);
            return -1;
        }
        ins_buffer = t;
    }

    int r = insert_memory_logic(arena, str, pos, ins.size);
    if (r != 0) {
        if (local_arena != NULL) {
            local_arena_alloc_reset(local_arena);
        }
        return r;
    }

    memcpy(str->buffer+pos, ins_buffer, ins.size);
    if (local_arena != NULL) {
        local_arena_alloc_reset(local_arena);
    }
    return 0;
}

static int overwrite_memory_logic(Arena* arena, String* str, size_t pos, size_t len) {
    size_t rm_len = pos+len > str->size ? str->size - pos : len;
    return _resize_and_shift_memory_logic(arena, str, pos, rm_len, len);
}

int string_overwrite_vfmt(Arena* arena, String* str, size_t pos, const char* fmt, va_list args) {
    va_list args2;

    if (str == NULL || str->buffer == NULL || pos > str->size || fmt == NULL) {
        return -1;
    }

    char fmt_buffer[128];
    va_copy(args2, args);
    int n = vsnprintf(fmt_buffer, sizeof(fmt_buffer), fmt, args2);
    va_end(args2);
    if (n<0) {
        return -1;
    }
    size_t len = (size_t) n;

    int r = overwrite_memory_logic(arena, str, pos, len);
    if (r != 0) {
        return r;
    }

    if (len < sizeof(fmt_buffer)) {
        memcpy(str->buffer+pos, fmt_buffer, len);
    }
    else {
        char save_char = str->buffer[pos+len];
        va_copy(args2, args);
        int m = vsnprintf(str->buffer+pos, len+1, fmt, args2);
        va_end(args2);
        // Note: the string has been mutated and if error => string is dirty
        if (m < 0 || (size_t) m != len) {
            return -1;
        }
        str->buffer[pos+len] = save_char;
    }
    return 0;
}

int string_overwrite_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = string_overwrite_vfmt(arena, str, pos, fmt, ap);
    va_end(ap);
    return r;
}

int string_overwrite_sv(Arena* arena, String* str, size_t pos, StringView sv) {
    if (str == NULL || str->buffer == NULL || pos > str->size || sv.buffer == NULL) {
        return -1;
    }

    LocalArena* local_arena = NULL;
    const char* sv_buffer = sv.buffer;

    // If the StringView is partly after where we insert
    // Copy to a side buffer
    if ((uintptr_t)str->buffer < (uintptr_t)sv.buffer + sv.size &&
        (uintptr_t)sv.buffer < (uintptr_t)str->buffer + str->size) {

        local_arena = local_arena_alloc_create();
        if (local_arena == NULL) {
            return -1;
        }

        // TODO: Optimization ? Only copy the part intersecting what moves
        char* t = (char*)arena_alloc_push_struct(local_arena->arena, (void*)sv.buffer, sv.size);
        if (t == NULL) {
            local_arena_alloc_reset(local_arena);
            return -1;
        }
        sv_buffer = t;
    }

    int r = overwrite_memory_logic(arena, str, pos, sv.size);
    if (r != 0) {
        if (local_arena != NULL) {
            local_arena_alloc_reset(local_arena);
        }
        return r;
    }

    memcpy(str->buffer+pos, sv_buffer, sv.size);
    if (local_arena != NULL) {
        local_arena_alloc_reset(local_arena);
    }
    return 0;
}

int string_erase(String* str, size_t pos, size_t len) {
    if ((str == NULL) || (str->buffer == NULL)) {
        return -1;
    }
    if (pos >= str->size) {
        return -1;
    }
    if (pos+len > str->size) {
        return -1;
    }

    memmove(str->buffer+pos, str->buffer+pos+len, str->size - (pos+len)+1);
    str->size -= len;
    return 0;
}

static int erase_and_insert_memory_logic(Arena* arena, String* str, size_t pos, size_t rm_len, size_t insert_len) {
    return _resize_and_shift_memory_logic(arena, str, pos, rm_len, insert_len);
}

int string_erase_and_insert_sv(Arena* arena, String* str, size_t pos, size_t len, StringView sv) {
    if (str == NULL || str->buffer == NULL || sv.buffer == NULL || pos+len > str->size) {
        return -1;
    }

    if (len == 0) {
        if (pos == 0) {
            return string_prepend_sv(arena, str, sv);
        }
        if (pos == str->size) {
            return string_append_sv(arena, str, sv);
        }
        return string_insert_sv(arena, str, pos, sv);
    }

    LocalArena* local_arena = NULL;
    const char* sv_buffer = sv.buffer;

    // If the StringView is partly after where we insert
    // Copy to a side buffer
    if ((uintptr_t)str->buffer < (uintptr_t)sv.buffer + sv.size &&
        (uintptr_t)sv.buffer < (uintptr_t)str->buffer + str->size) {

        local_arena = local_arena_alloc_create();
        if (local_arena == NULL) {
            return -1;
        }

        // TODO: Optimization ? Only copy the part intersecting what moves
        char* t = (char*)arena_alloc_push_struct(local_arena->arena, (void*)sv.buffer, sv.size);
        if (t == NULL) {
            local_arena_alloc_reset(local_arena);
            return -1;
        }
        sv_buffer = t;
    }

    // Check the size difference between what is removed and what is added
    if (len == sv.size) {
        memcpy(str->buffer + pos, sv_buffer, sv.size);
        if (local_arena != NULL) {
            local_arena_alloc_reset(local_arena);
        }
        return 0;
    }

    int r = erase_and_insert_memory_logic(arena, str, pos, len, sv.size);
    if (r != 0) {
        if (local_arena != NULL) {
            local_arena_alloc_reset(local_arena);
        }
        return r;
    }

    memcpy(str->buffer+pos, sv_buffer, sv.size);
    if (local_arena != NULL) {
        local_arena_alloc_reset(local_arena);
    }
    return 0;
}

int string_erase_and_insert_vfmt(Arena* arena, String* str, size_t pos, size_t len, const char* fmt, va_list args) {
    if (str == NULL || str->buffer == NULL) {
        return -1;
    }
    va_list args2;

    va_copy(args2, args);
    if (len == 0) {
        int r;
        if (pos == 0) {
            r = string_prepend_vfmt(arena, str, fmt, args2);
        } else if (pos == str->size) {
            r = string_append_vfmt(arena, str, fmt, args2);
        } else {
            r = string_insert_vfmt(arena, str, pos, fmt, args2);
        }
        va_end(args2);
        return r;
    }

    char fmt_buffer[128];
    int n = vsnprintf(fmt_buffer, sizeof(fmt_buffer), fmt, args2);
    va_end(args2);
    if (n<0) {
        return -1;
    }
    size_t insert_len = (size_t) n;

    // Check the size difference between what is removed and what is added
    if (insert_len == len) {
        if (insert_len < sizeof(fmt_buffer)) {
            memcpy(str->buffer+pos, fmt_buffer, len);
        }
        else {
            char save_char = str->buffer[pos+len];
            va_copy(args2, args);
            int m = vsnprintf(str->buffer+pos, len+1, fmt, args2);
            va_end(args2);
            if (m < 0 || (size_t) m != len) {
                return -1;
            }
            str->buffer[pos+len] = save_char;
        }
        return 0;
    }

    int r = erase_and_insert_memory_logic(arena, str, pos, len, insert_len);

    if (r != 0) {
        return -1;
    }

    if (insert_len < sizeof(fmt_buffer)) {
        memcpy(str->buffer+pos, fmt_buffer, insert_len);
    }
    else {
        char save_char = str->buffer[pos+insert_len];
        va_copy(args2, args);
        int m = vsnprintf(str->buffer+pos, insert_len+1, fmt, args2);
        va_end(args2);
        // Note: This breaks str immutability on error
        if (m < 0 || (size_t) m != insert_len) {
            return -1;
        }
        str->buffer[pos+insert_len] = save_char;
    }
    return 0;
}

int string_erase_and_insert_fmt(Arena* arena, String* str, size_t pos, size_t len, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = string_erase_and_insert_vfmt(arena, str, pos, len, fmt, ap);
    va_end(ap);
    return r;
}

int string_replace_first(Arena* arena, String* str, StringView target, StringView replacement) {
    if (str == NULL || str->buffer == NULL) {
        return -1;
    }
    StringView sv = sv_from_string(*str);
    size_t pos = sv_find(sv, target);
    if (pos == sv.size) {
        return -1;
    }
    return string_erase_and_insert_sv(arena, str, pos, target.size, replacement);
}

int string_replace_last(Arena* arena, String* str, StringView target, StringView replacement) {
    if (str == NULL || str->buffer == NULL) {
        return -1;
    }
    StringView sv = sv_from_string(*str);
    size_t pos = sv_rfind(sv, target);
    if (pos == sv.size) {
        return -1;
    }
    return string_erase_and_insert_sv(arena, str, pos, target.size, replacement);
}

int string_replace_all(Arena* arena, String* str, StringView target, StringView replacement) {
    if (str == NULL || str->buffer == NULL) {
        return -1;
    }
    if (target.size == 0) {
        return 0;
    }

    size_t count = 0;
    StringView sv = sv_from_string(*str);
    StringView search_sv = sv;

    // Count how many instances of the target
    // TODO: We could cache the positions of the targets to replace
    while (search_sv.size >= target.size) {
        size_t p = sv_find(search_sv, target);
        if (p == search_sv.size) {
            break;
        }
        count++;
        search_sv = sv_truncate_front(search_sv, p+target.size);
    }

    if (count == 0) {
        return 0;
    }

    // Get the new size of the string depending on if it grows or shrinks
    size_t size_difference = 0;
    unsigned char grows = replacement.size > target.size;

    if (!grows) {
        size_difference = count * (target.size - replacement.size);
    }
    else {
        size_difference = count * (replacement.size - target.size);
        if (SIZE_MAX - str->size < size_difference) return -1;
    }

    size_t new_size = grows ? str->size + size_difference : str->size - size_difference;

    // Allocate a new string
    size_t new_capacity = _get_new_capacity(0, new_size);
    char* new_buffer = (char*)arena_alloc_push(arena, new_capacity);
    if (new_buffer == NULL)  {
        return -1;
    }

    // Go through the string again and copy to new buffer
    // and insert the replacements
    search_sv = sv;
    char* write_p = new_buffer;

    while(search_sv.size >= 0) {
        size_t p = sv_find(search_sv, target);

        if (p == search_sv.size) {
            // Nothing more found
            // But we still need to copy stuff
            memcpy(write_p, search_sv.buffer, search_sv.size);
            write_p += search_sv.size;
            break;
        }

        // Only copy what's before if there is something before...
        if (p > 0) {
            memcpy(write_p, search_sv.buffer, p);
            write_p += p;
        }

        if (replacement.size > 0 && replacement.buffer != NULL) {
            memcpy(write_p, replacement.buffer, replacement.size);
            write_p += replacement.size;
        }

        search_sv = sv_truncate_front(search_sv, p+target.size);
    }

    str->buffer = new_buffer;
    str->size = new_size;
    str->capacity = new_capacity;

    return 0;
}

int string_clear(String* str) {
    if ((str == NULL) || (str->buffer == NULL)) return -1;
    str->size = 0;
    str->buffer[0] = '\0';
    return 0;
}

String string_deep_copy(Arena* arena, const String* str) {
    if ((str == NULL) || (str->buffer == NULL)) return (String){0};

    size_t checkpoint = arena_alloc_checkpoint(arena);
    void* p = arena_alloc_push_struct(arena, (void*)str->buffer, str->size+1);
    if (p == NULL) return (String){0};

    if (str->capacity > str->size+1) {
        void* tmp = arena_alloc_push_unaligned(arena, str->capacity - (str->size+1));
        if (tmp == NULL) {
            arena_alloc_restore(arena, checkpoint);
            return (String){0};
        }
    }

    return (String){
        .buffer = (char*)p,
        .size = str->size,
        .capacity = str->capacity,
    };
}

int string_reserve(Arena* arena, String* str, size_t new_capacity) {
    if (str == NULL || str->buffer == NULL) {
        return -1;
    }

    if (new_capacity < str->capacity) {
        return -1;
    }

    const char* arena_top = (const char*)arena_alloc_used_location(arena);
    if ((uintptr_t)arena_top == (uintptr_t)str->buffer + str->capacity) {
        void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
        if (t == NULL) return -1;
    }
    else {
        char* t = (char*)arena_alloc_push(arena, new_capacity);
        if (t == NULL) return -1;
        memcpy(t, str->buffer, str->size+1);
        str->buffer = t;
    }
    str->capacity = new_capacity;
    return 0;
}

int string_shrink_to_fit(Arena* arena, String* str) {
    if (str == NULL || str->buffer == NULL) {
        return -1;
    }
    void* arena_top = arena_alloc_used_location(arena);
    if (arena_top == NULL) {
        return -1;
    }
    // Capacity not changed because not on top of arena
    if ((uintptr_t)arena_top != (uintptr_t)str->buffer + str->capacity) {
        // NOTE: Not sure what to return here...
        return -1;
    }
    void* t = arena_alloc_pop_to(arena, str->buffer + str->size+1);
    if (t == NULL) {
        return -1;
    }
    str->capacity = str->size + 1;
    return 0;
}

void string_debug_print(const String* s) {
    printf("String Debug Info:\n");
    printf(" String:      %p\n",             s);
    if (s == NULL) return;
    if (s->buffer != NULL) {
    printf("  buffer:     %p\n",             s->buffer);
    printf("  size:       %zu bytes   %p\n", s->size, s->buffer+s->size+1);
    printf("  capacity:   %zu bytes   %p\n", s->capacity, s->buffer+s->capacity);
    printf("  free:       %zu bytes\n",      s->capacity - s->size - 1);
    } else {
    printf("  buffer:     NULL\n");
    }
}

void string_print(const String* s) {
    if (s == NULL || s->buffer == NULL) {
        printf("NULL String!\n");
        return;
    }
    printf("%s", s->buffer);
}

//==============================
// STRING VIEW
//==============================
StringView sv_from_buffer(const char* buffer, size_t len) {
    return buffer ? (StringView){ .buffer = buffer, .size = len } : (StringView){0};
}

StringView sv_from_sv(StringView sv) {
    return sv;
}

StringView sv_from_cstr(const char* cstr) {
    return cstr ? (StringView){ .buffer = cstr, .size = strlen(cstr) } : (StringView){0};
}

StringView sv_from_string(String str) {
    return str.buffer ? (StringView){ .buffer = str.buffer, .size = str.size } : (StringView){0};
}

StringView sv_slice_sv(StringView sv, size_t start, size_t len) {
    if ( (start > sv.size) || (len > sv.size-start) ) {
        return (StringView){0};
    }
    return (StringView) {
        .buffer = sv.buffer+start,
        .size = len,
    };
}

StringView sv_truncate_front(StringView sv, size_t len) {
    if (len > sv.size) return (StringView){0};
    return (StringView){
        .buffer = sv.buffer+len,
        .size = sv.size-len,
    };
}

StringView sv_truncate_back(StringView sv, size_t len) {
    if (len > sv.size) return (StringView){0};
    return (StringView){
        .buffer = sv.buffer,
        .size = sv.size-len,
    };
}

StringView sv_trim_front(StringView sv) {
    /*
     *  '\t': 9
     *  '\n': 10
     *  '\v': 11
     *  '\f': 12
     *  '\r': 13
     *  ' ': 32
     */
    if (sv.size == 0) return sv;
    size_t i = 0;
    while (i < sv.size) {
        unsigned char c = sv.buffer[i];
        if (!((c >= 9 && c <= 13) || c == 32)) {
            break;
        }
        i++;
    }
    return (StringView){
        .buffer = sv.buffer+i,
        .size = sv.size-i,
    };
}

StringView sv_trim_back(StringView sv) {
    /*
     *  '\t': 9
     *  '\n': 10
     *  '\v': 11
     *  '\f': 12
     *  '\r': 13
     *  ' ': 32
     */
    if (sv.size == 0) return sv;
    while (sv.size > 0) {
        unsigned char c = sv.buffer[sv.size-1];
        if (!((c>=9 && c<=13) || (c == 32))) {
            break;
        }
        sv.size--;
    }
    return sv;
}

StringView sv_chop_by_delim_sv(StringView* sv, StringView delim) {
    if (sv == NULL || sv->buffer == NULL || delim.buffer == NULL) {
        return (StringView){0};
    }
    if (delim.size == 0) {
        return (StringView){.buffer=sv->buffer, .size=0};
    }
    size_t pos = sv_find(*sv, delim);
    // TODO: I think this if is useless
    if (pos == sv->size) {
        StringView out = sv_from_sv(*sv);
        sv->buffer += sv->size;
        sv->size = 0;
        return out;
    }
    sv->buffer += pos+delim.size;
    sv->size -= (pos+delim.size);
    return (StringView){
        .buffer = sv->buffer-(pos+delim.size),
        .size = pos,
    };
}

StringView sv_chop_by(StringView* sv, size_t pos) {
    if (sv == NULL) {
        return (StringView){0};
    }
    if (pos > sv->size) {
        return (StringView){0};
    }
    sv->buffer += pos;
    sv->size -= pos;
    return (StringView){
        .buffer = sv->buffer - pos,
        .size = pos,
    };
}

bool sv_equal(StringView sv1, StringView sv2) {
    if (sv1.size != sv2.size) return false;

    return _compare(sv1.buffer, sv2.buffer, sv1.size) == 0;
}

int sv_compare(const StringView* sv1, const StringView* sv2) {
    // <0 if sv1 < sv2
    // =0 if sv1 == sv2
    // >0 if sv1 > sv2
    size_t len = sv1->size<sv2->size ? sv1->size : sv2->size;

    int cmp = _compare(sv1->buffer, sv2->buffer, len);
    if (cmp != 0) return cmp;

    if (sv1->size < sv2->size) return -1;
    if (sv1->size > sv2->size) return 1;
    return 0;
}

bool sv_starts_with(StringView sv, StringView pre) {
    if (sv.buffer == NULL || pre.buffer == NULL || pre.size > sv.size) {
        return false;
    }

    return _compare(sv.buffer, pre.buffer, pre.size) == 0;
}

bool sv_starts_with_char(StringView sv, char c) {
    if (sv.size == 0) {
        return false;
    }
    return sv.buffer[0] == c;
}

bool sv_ends_with(StringView sv, StringView post) {
    if (sv.buffer == NULL || post.buffer == NULL || sv.size < post.size) {
        return false;
    }

    return _compare(sv.buffer + sv.size-post.size, post.buffer, post.size) == 0;
}

static size_t BMH_search(StringView haystack, StringView needle) {
    /*
     * Boyer-Moore-Horspool fallback for better perf than naive if not on linux.
     * https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm
     */

    // Preprocess pattern
    size_t table[256];
    for (size_t i = 0; i<256; i++) {
        table[i] = needle.size;
    }
    const unsigned char* h_buf = (const unsigned char*) haystack.buffer;
    const unsigned char* n_buf = (const unsigned char*) needle.buffer;

    for (size_t i = 0; i<needle.size-1; i++) {
        table[n_buf[i]] = needle.size - 1 - i;
    }

    size_t skip = 0;

    while (haystack.size - skip >= needle.size) {
        if (memcmp(&h_buf[skip], n_buf, needle.size) == 0) {
            return skip;
        }
        skip += table[h_buf[skip+needle.size-1]];
    }
    return haystack.size;
}

static size_t BMH_rsearch(StringView haystack, StringView needle) {
    /*
     * Boyer-Moore-Horspool fallback for better perf than naive if not on linux.
     * https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm
     */
    // Preprocess pattern
    size_t table[256];
    for (size_t i = 0; i<256; i++) {
        table[i] = needle.size;
    }

    const unsigned char* h_buf = (const unsigned char*) haystack.buffer;
    const unsigned char* n_buf = (const unsigned char*) needle.buffer;

    for (size_t i = needle.size - 1; i >= 1; i--) {
        table[n_buf[i]] = i;
    }

    size_t skip = haystack.size - needle.size;

    while (skip <= haystack.size - needle.size) {
        if (memcmp(&h_buf[skip], n_buf, needle.size) == 0) {
            return skip;
        }
        skip -= table[h_buf[skip]];
    }

    return haystack.size;
}

// TODO(alex): Now wiki had an implementation of BMH, apparently, there is something even
// better called two way search. TBC
size_t sv_find(StringView haystack, StringView needle) {
    if (haystack.buffer == NULL || needle.buffer == NULL) {
        return haystack.size;
    }
    if (needle.size == 0) return 0;
    if (needle.size > haystack.size) return haystack.size;

#if defined(__linux__) || defined(_GNU_SOURCE) || defined(__GLIBC__)

    const void* p = memmem(haystack.buffer, haystack.size, needle.buffer, needle.size);
    if (p != NULL) {
        return (size_t)((const char*)p - haystack.buffer);
    }
    return haystack.size;

#else

    const unsigned char* h = (const unsigned char*) haystack.buffer;
    const unsigned char* n = (const unsigned char*) needle.buffer;

    if (needle.size == 1) {
        const unsigned char* match = (const unsigned char*) memchr((void*)h, n[0], haystack.size);
        return match != NULL ? (size_t)(match - h) : haystack.size;
    }
    else if (needle.size > 6) {         // Just a "random" number, BMH is better for larger needles
        return BMH_search(haystack, needle);
    }
    else {
        size_t remaining = haystack.size - needle.size + 1;
        const unsigned char* p = h;

        while (remaining > 0) {
            const unsigned char* match = (const unsigned char*) memchr((void*)p, n[0], remaining);
            if (match == NULL) {
                break;
            }
            if (memcmp(match, n, needle.size) == 0) {
                return (size_t)(match - h);
            }
            size_t advance = (size_t)(match - p) + 1;
            p = match+1;
            remaining -= advance;
        }
        return haystack.size;
    }
#endif
}

size_t sv_rfind(StringView haystack, StringView needle) {
    if (haystack.buffer == NULL || needle.buffer == NULL) {
        return haystack.size;
    }
    if (needle.size == 0) return haystack.size;
    if (needle.size > haystack.size) return haystack.size;

    const unsigned char* h = (const unsigned char*) haystack.buffer;
    const unsigned char* n = (const unsigned char*) needle.buffer;

#if defined(__linux__) || defined(_GNU_SOURCE) || defined(__GLIBC__)
    if (needle.size == 1) {
        const unsigned char* match = (const unsigned char*) memrchr((void*)h, n[0], haystack.size);
        return match != NULL ? (size_t)(match-h) : haystack.size;
    }
    else if (needle.size > 6) {
        return BMH_rsearch(haystack, needle);
    }
    else {
        const unsigned char* p = h;
        size_t remaining = haystack.size - needle.size + 1;
        while (true) {
            const unsigned char* match = (const unsigned char*) memrchr((void*)h, n[0], remaining);
            if (match == NULL) {
                break;
            }
            if (memcmp(match, n, needle.size) == 0) {
                return (size_t)(match - h);
            }
            if (match == h) {
                break;
            }
            remaining = (size_t)(match - h);
        }
        return haystack.size;
    }
#else
    return BMH_rsearch(haystack, needle);

    // size_t i = haystack.size-needle.size;
    // while (true) {
    //     if ((h[i] == n[0]) && (memcmp(h+i , n, needle.size) == 0)) {
    //         return i;
    //     }
    //     if (i == 0) {
    //         break;
    //     }
    //     i--;
    // }
    // return haystack.size;
#endif
}

StringView sv_file_extension(StringView sv) {
    size_t p1 = sv_rfind(sv, sv_from_cstr("/"));
    StringView t = sv;
    if (p1 != sv.size) {
        t = sv_truncate_front(sv, p1);
    }

    size_t p2 = sv_rfind(t, sv_from_cstr("."));
    if (p2 == t.size) {
        return (StringView){0};
    }
    return (StringView){.buffer=sv.buffer+p1+p2, .size=sv.size-p1-p2};
}

StringView sv_file_name(StringView sv) {
    size_t pos = sv_rfind(sv, sv_from_cstr("/"));
    if (pos == sv.size) {
        pos = 0;
    }
    return (StringView){.buffer=sv.buffer+pos+1, .size=sv.size-(pos+1)};
}

StringView sv_directory_name(StringView sv){
    size_t pos = sv_rfind(sv, sv_from_cstr("/"));
    if (pos == sv.size) {
        return sv;
    }
    return (StringView){.buffer=sv.buffer, .size=pos+1};
}

const char* sv_as_cstr(Arena* arena, StringView sv) {
    if (sv.size == 0) {
        return "";
    }
    char* t = (char*)arena_alloc_push_struct(arena, (void*)sv.buffer, sv.size);
    if (t == NULL) {
        return "";
    }
    arena_alloc_push_unaligned(arena, 1);
    t[sv.size] = '\0';
    return t;
}

enum CharType {
    SV_CHAR_SPACE = 0x1,
    SV_CHAR_DIGIT = 0x2,
    SV_CHAR_ALPHA = 0x4,
    SV_CHAR_DOT   = 0x8,
    SV_CHAR_NEG   = 0x10,
};

static uint8_t LUT[256] = {
    [' ']  = SV_CHAR_SPACE,
    ['\t'] = SV_CHAR_SPACE,
    ['\n'] = SV_CHAR_SPACE,
    ['\r'] = SV_CHAR_SPACE,
    ['\v'] = SV_CHAR_SPACE,
    ['\f'] = SV_CHAR_SPACE,

    ['0']  = SV_CHAR_DIGIT,
    ['1']  = SV_CHAR_DIGIT,
    ['2']  = SV_CHAR_DIGIT,
    ['3']  = SV_CHAR_DIGIT,
    ['4']  = SV_CHAR_DIGIT,
    ['5']  = SV_CHAR_DIGIT,
    ['6']  = SV_CHAR_DIGIT,
    ['7']  = SV_CHAR_DIGIT,
    ['8']  = SV_CHAR_DIGIT,
    ['9']  = SV_CHAR_DIGIT,

    ['-']  = SV_CHAR_NEG,

    // TODO continue when needed
};

static void _sv_trim_front(StringView* sv) {
    size_t i = 0;
    while ((i < sv->size) && (LUT[sv->buffer[0]] & SV_CHAR_SPACE)) {
        sv->buffer++;
        i++;
    }
    sv->size -= i;
}

int sv_parse_u32(StringView* sv, uint32_t* out) {
    if (sv == NULL || sv->buffer == NULL) {
        return -1;
    }

    _sv_trim_front(sv);

    uint32_t v = 0;
    unsigned char i = 0;
    // UINT32_MAX = 4_294_967_295
    while ( (i<sv->size) && (LUT[sv->buffer[0]] & SV_CHAR_DIGIT)) {
        if ((i==9) &&
            (   ( v >  (uint32_t)(UINT32_MAX / 10)) ||
                ((v == (uint32_t)(UINT32_MAX / 10)) && ((sv->buffer[0]-'0') > 5))
            )
           ) {
            return -1;
        }
        v = v*10 + (sv->buffer[0]-'0');
        sv->buffer++;
        i++;
    }
    sv->size -= i;
    *out = v;

    return 0;
}

int sv_parse_s32(StringView* sv, int32_t* out) {
    if (sv == NULL || sv->buffer == NULL) {
        return -1;
    }

    _sv_trim_front(sv);
    if (sv->size == 0) {
        return -1;
    }

    int sign = 1;
    if (LUT[sv->buffer[0]] & SV_CHAR_NEG) {
        sign = -1;
        sv->buffer++;
        sv->size--;
    }

    uint32_t v = 0;
    unsigned char i = 0;
    // INT32_MAX =  2_147_483_647
    // INT32_MIN = -2_147_483_648
    while ( (i<sv->size) && (LUT[sv->buffer[0]] & SV_CHAR_DIGIT)) {
        if (i==9) {
            if ( (sign == 1) && ( (v>INT32_MAX/10) || ((v==INT32_MAX/10) && (sv->buffer[0]-'0'>7)) ) ) {
                return -1;
            }
            else if ((sign == -1) && ( (-1*v<INT32_MIN/10) || ((-1*v==INT32_MIN/10) && (sv->buffer[0]-'0'>8)) )){
                return -1;
            }
        }
        v = v*10 + (sv->buffer[0]-'0');
        sv->buffer++;
        i++;
    }
    sv->size -= i;

    *out = sign * v;
    return 0;
}

// int sv_parse_f32(StringView* sv, float* out);

void sv_print(StringView sv) {
    if (sv.buffer) {
        printf("%.*s", (int)sv.size, sv.buffer);
    }
}

void sv_debug_print(StringView sv) {
    printf("StringView Debug Info:\n");
    printf("  buffer:     %p\n",             sv.buffer);
    printf("  size:       %zu bytes   %p\n", sv.size, sv.buffer+sv.size);
}
