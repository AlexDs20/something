#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "memory/allocators.h"
#include "libs/ads_string.h"

static size_t get_new_capacity(size_t new_size) {
    return new_size >= 16 ? (2*new_size)+1 : 16;
}

static int compare(const void* v1, const void* v2, size_t len) {
    const char* s1 = (const char*)v1;
    const char* s2 = (const char*)v2;
    while (len-- > 0) {
        if (*s1++ != *s2++) {
            return s1[-1] < s2[-1] ? -1 : 1;
        }
    }
    return 0;
}

typedef void* (*MemCopyFunc)(
    void* dest,
    const void* src,
    size_t n
);


//==============================
// STRING
//==============================
String string_init_empty(Arena* arena, size_t capacity) {
    String str = {0};

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
    String str = {0};
    if (fmt == NULL) {
        return str;
    }

    // Source: https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Variable-Number-of-Arguments.html
    va_list args2;
    va_copy(args2, args);

    int n = vsnprintf(NULL, 0, fmt, args2);
    va_end(args2);

    if (n < 0) {
        return (String){0};
    }

    size_t len = (size_t)n;

    size_t capacity = get_new_capacity(len);
    str.buffer = (char*) arena_alloc_push(arena, capacity);
    if (str.buffer == NULL) {
        return (String){0};
    }
    va_copy(args2, args);
    vsnprintf(str.buffer, len+1, fmt, args2);          // Write len+1 elements including '\0'
    va_end(args2);

    str.capacity = capacity;
    str.size = len;
    return str;
}

String string_init_sv(Arena* arena, StringView sv) {
    String str = {0};

    if (sv.buffer == NULL) {
        return str;
    }

    str.capacity = get_new_capacity(sv.size);
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

int string_append_vfmt(Arena* arena, String* str, const char* fmt, va_list args){
    va_list args2;
    va_copy(args2, args);

    //==================================================
    // Source: https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Variable-Number-of-Arguments.html
    if (str == NULL || str->buffer == NULL || fmt == NULL) {
        return -1;
    }

    // Sets ap to point before the first additional argument
    int n = vsnprintf(NULL, 0, fmt, args2);        // Return the "written" size without '\0'
    va_end(args2);

    if (n < 0) {
        return -1;
    }

    size_t len = (size_t)n;

    if (str->size + len + 1 > str->capacity) {
        void* arena_top = arena_alloc_used_location(arena);
        if (arena_top == NULL) return -1;

        size_t new_size = str->size + len;
        size_t new_capacity = get_new_capacity(new_size);

        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) {
                return -1;
            }
        }
        else {
            char* t = (char*)arena_alloc_push(arena, new_capacity);
            if (t == NULL) {
                return -1;
            }
            memcpy(t, str->buffer, str->size);
            str->buffer = t;
        }
        str->capacity = new_capacity;
    }

    va_copy(args2, args);
    vsnprintf(str->buffer+str->size, len+1, fmt, args2);
    va_end(args2);

    str->size += len;
    str->buffer[str->size] = '\0';

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

    if (str->size + append.size + 1 > str->capacity) {
        void* arena_top = arena_alloc_used_location(arena);
        if (arena_top == NULL) return -1;

        size_t new_size = str->size + append.size;
        size_t new_capacity = get_new_capacity(new_size);

        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) {
                return -1;
            }
        }
        else {
            char* t = (char*)arena_alloc_push(arena, new_capacity);
            if (t == NULL) {
                return -1;
            }
            memcpy(t, str->buffer, str->size);
            str->buffer = t;
        }
        str->capacity = new_capacity;
    }

    memcpy(str->buffer+str->size, append.buffer, append.size);
    str->size += append.size;
    str->buffer[str->size] = '\0';

    return 0;
}

int string_prepend_vfmt(Arena* arena, String* str, const char* fmt, va_list args) {
    va_list args2;
    va_copy(args2, args);

    //==============================
    if (str == NULL || str->buffer == NULL || fmt == NULL) {
        return -1;
    }

    int n = vsnprintf(NULL, 0, fmt, args2);
    va_end(args2);

    if (n<0) {
        return -1;
    }

    size_t len = (size_t) n;

    const char save_char = str->buffer[0];

    if (str->size+1+len > str->capacity) {
        size_t new_capacity = get_new_capacity(str->size+len);
        const char* arena_top = (char*)arena_alloc_used_location(arena);
        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) return -1;
            memmove(str->buffer+len, str->buffer, str->size+1);
        }
        else {
            char* t = (char*)arena_alloc_push(arena, new_capacity);
            if (t == NULL) return -1;
            memcpy(t+len, str->buffer, str->size+1);
            str->buffer = t;
        }
        str->capacity = new_capacity;
    }
    else {
        memmove(str->buffer+len, str->buffer, str->size+1);
    }

    va_copy(args2, args);
    vsnprintf(str->buffer, len+1, fmt, args2);
    va_end(args2);

    str->buffer[len] = save_char;
    str->size += len;
    //==============================
    return 0;
}

int string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = string_prepend_vfmt(arena, str, fmt, ap);
    va_end(ap);
    return r;
}

// TODO(alex): Handle if the StringView is in the current string!!
int string_prepend_sv(Arena* arena, String* str, StringView pre) {
    size_t new_size;
    char* arena_top;
    char* tmp;

    if ((str == NULL) || (str->buffer == NULL) || (pre.buffer == NULL)) {
        return -1;
    }

    LocalArena* local_arena;
    const char* pre_buffer = pre.buffer;

    // If the StringView is a part the string we prepend
    // Copy to a side buffer
    if ((str->buffer<=pre.buffer && pre.buffer<str->buffer+str->size) ||
        (str->buffer<=pre.buffer+pre.size && pre.buffer+pre.size<str->buffer+str->size)) {

        // Note that because the string owns the memory the StringView should be fully contained in the String
        // If that is not the case -> user error!
        if (pre.buffer < str->buffer || str->buffer+str->size<pre.buffer+pre.size) {
            return -1;
        }

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

    new_size = str->size + pre.size;

    // Need to realloc
    if (new_size + 1 > str->capacity) {
        size_t new_capacity = get_new_capacity(new_size);

        arena_top = (char*)arena_alloc_used_location(arena);
        if (arena_top == NULL) {
            if (local_arena == NULL) {
                local_arena_alloc_reset(local_arena);
            }
            return -1;
        }

        if (arena_top == str->buffer + str->capacity) {
            size_t bytes_needed = new_capacity - str->capacity;
            tmp = (char*)arena_alloc_push_unaligned(arena, bytes_needed);
            if (tmp == NULL) {
                if (local_arena == NULL) {
                    local_arena_alloc_reset(local_arena);
                }
                return -1;
            }
            memmove(str->buffer+pre.size, str->buffer, str->size+1);
        }
        else {
            tmp = (char*)arena_alloc_push(arena, new_capacity);
            if (tmp == NULL) {
                if (local_arena == NULL) {
                    local_arena_alloc_reset(local_arena);
                }
                return -1;
            }
            memcpy(tmp+pre.size, str->buffer, str->size+1);
            str->buffer = tmp;
        }
        str->capacity = new_capacity;
    }
    else {
        memmove(str->buffer+pre.size, str->buffer, str->size+1);
    }

    memcpy(str->buffer, pre_buffer, pre.size);
    if (local_arena == NULL) {
        local_arena_alloc_reset(local_arena);
    }
    str->size = new_size;
    return 0;
}

int string_insert_vfmt(Arena* arena, String* str, size_t pos, const char* fmt, va_list args) {
    va_list args2;
    va_copy(args2, args);

    if (str == NULL || str->buffer == NULL || fmt == NULL || pos > str->size) {
        return -1;
    }

    if (pos == 0) {
        int r = string_prepend_vfmt(arena, str, fmt, args2);
        va_end(args2);
        return r;
    } else if (pos == str->size) {
        int r = string_append_vfmt(arena, str, fmt, args2);
        va_end(args2);
        return r;
    }

    int n = vsnprintf(NULL, 0, fmt, args2);
    va_end(args2);

    if (n<0) {
        return -1;
    }

    size_t len = (size_t) n;

    // Save first char of what is after the insert because vsnprintf always puts a \0 ...
    char save_char = str->buffer[pos];

    if (str->size+1+len > str->capacity) {
        size_t new_capacity = get_new_capacity(str->size+len);
        char* arena_top = (char*)arena_alloc_used_location(arena);
        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) {
                return -1;
            }
            memmove(str->buffer+pos+len, str->buffer+pos, str->size+1 - pos);
        }
        else {
            char* t = (char*) arena_alloc_push(arena, new_capacity);
            if (t == NULL) {
                return -1;
            }
            memcpy(t, str->buffer, pos);
            memcpy(t+pos+len, str->buffer+pos, str->size+1 - pos);
            str->buffer = t;
        }
        str->capacity = new_capacity;
    }
    else {
        memmove(str->buffer+pos+len, str->buffer+pos, str->size+1 - pos);
    }


    va_copy(args2, args);
    vsnprintf(str->buffer+pos, len+1, fmt, args2);
    va_end(args2);
    str->buffer[pos+len] = save_char;
    str->size += len;
    return 0;
}

int string_insert_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int r = string_prepend_vfmt(arena, str, fmt, ap);
    va_end(ap);
    return r;
}

// TODO(alex): Handle if StringView intersect the String
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


    size_t new_size = str->size + ins.size;

    if (str->size+1+ins.size > str->capacity) {
        size_t new_capacity = get_new_capacity(new_size);
        const char* arena_top = (char*) arena_alloc_used_location(arena);
        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) {
                return -1;
            }
            memmove(str->buffer+pos+ins.size, str->buffer+pos, str->size+1 - pos);
        }
        else {
            char* t = (char*)arena_alloc_push(arena, new_capacity);
            if (t == NULL) {
                return -1;
            }
            memcpy(t, str->buffer, pos);
            memcpy(t+pos+ins.size, str->buffer+pos, str->size+1 - pos);
            str->buffer = t;
        }
        str->capacity = new_capacity;
    }
    else {
        memmove(str->buffer+pos+ins.size, str->buffer+pos, str->size+1 - pos);
    }

    memcpy(str->buffer+pos, ins.buffer, ins.size);

    str->size += ins.size;
    return 0;
}

// TODO(alex): Handle if StringView intersect the String
int string_overwrite_vfmt(Arena* arena, String* str, size_t pos, const char* fmt, va_list args) {
    va_list args2;

    if (str == NULL || str->buffer == NULL || pos > str->size || fmt == NULL) {
        return -1;
    }

    va_copy(args2, args);
    int n = vsnprintf(NULL, 0, fmt, args2);
    va_end(args2);

    if (n<0) {
        return -1;
    }

    size_t len = (size_t) n;

    // Need to realloc
    if (pos+len+1 > str->capacity) {
        size_t new_size = pos+len;
        char* arena_top = (char*)arena_alloc_used_location(arena);
        size_t new_capacity = get_new_capacity(new_size);

        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) {
                return -1;
            }
        }
        else {
            char* t = (char*)arena_alloc_push(arena, new_capacity);
            if (t == NULL) {
                return -1;
            }
            memcpy(t, str->buffer, pos);
            str->buffer = t;
        }

        str->capacity = new_capacity;
        str->size = pos+len;
        str->buffer[str->size] = '\0';
    }
    else {
        // size_t end = pos+len > str->size ? pos+len : str->size;
        if (pos+len > str->size) {
            str->size = pos+len;
            str->buffer[pos+len] = '\0';
        }
    }

    char save_char = str->buffer[pos+len];

    va_copy(args2, args);
    vsnprintf(str->buffer+pos, len+1, fmt, args2);
    va_end(args2);
    str->buffer[pos+len] = save_char;
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

    if (pos+sv.size+1 > str->capacity) {
        size_t new_size = pos+sv.size;
        size_t new_capacity = get_new_capacity(new_size);
        char* arena_top = (char*)arena_alloc_used_location(arena);

        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) {
                return -1;
            }
        }
        else {
            char* t = (char*)arena_alloc_push(arena, new_capacity);
            if (t == NULL) {
                return -1;
            }
            memcpy(t, str->buffer, pos);
            str->buffer = t;
        }
        str->capacity = new_capacity;
        str->size = new_size;
    }
    else {
        // Enough capacity
        if (pos + sv.size > str->size) {
            str->size = pos + sv.size;
        }
    }
    memcpy(str->buffer+pos, sv.buffer, sv.size);
    str->buffer[str->size] = '\0';
    return 0;
}

int string_erase(String* str, size_t pos, size_t len) {
    if ((str == NULL) || (str->buffer == NULL)) {
        return -1;
    }
    if (pos < str->size) {
        if (pos+len >= str->size) {
            str->size = pos;
            str->buffer[pos] = '\0';
            return 0;
        }
        memmove(str->buffer+pos, str->buffer+pos+len, str->size - (pos+len));
        str->size -= len;
        str->buffer[str->size] = '\0';
        return 0;
    }
    return 0;
}

// TODO(alex): Handle if StringView intersect the String
static int erase_and_insert_main_logic(Arena* arena, String* str, size_t pos, size_t len, size_t insert_len) {
    // Check the size difference between what is removed and what is added
    int size_difference = len - insert_len;
    size_t new_size = str->size + size_difference;

    // If we grow beyond the current capacity => need more space
    if (str->size+1+size_difference > str->capacity) {
        size_t new_capacity = get_new_capacity(str->size+size_difference);
        char* arena_top = (char*)arena_alloc_used_location(arena);
        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) return -1;
            memmove(str->buffer+pos+insert_len, str->buffer+pos+len, str->size+1-(pos+len));
        }
        else {
            char* t = (char*)arena_alloc_push(arena, new_capacity);
            if (t == NULL) return -1;
            memcpy(t, str->buffer, pos);
            memcpy(t+pos+insert_len, str->buffer+pos+len, str->size+1-(pos+len));
            str->buffer = t;
        }
        str->capacity = new_capacity;
    }
    else {
        memmove(str->buffer+pos+insert_len, str->buffer+pos+len, str->size+1-(pos+len));
    }
    return 0;
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

    // Check the size difference between what is removed and what is added
    if (len == sv.size) {
        memcpy(str->buffer + pos, sv.buffer, sv.size);
        return 0;
    }

    int r = erase_and_insert_main_logic(arena, str, pos, len, sv.size);
    if (r != 0) {
        return r;
    }

    memcpy(str->buffer+pos, sv.buffer, sv.size);
    str->size += (sv.size-len);
    return 0;
}

int string_erase_and_insert_vfmt(Arena* arena, String* str, size_t pos, size_t len, const char* fmt, va_list args) {
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

    int n = vsnprintf(NULL, 0, fmt, args2);
    va_end(args2);
    if (n<0) {
        return -1;
    }
    size_t insert_len = (size_t) n;

    // Check the size difference between what is removed and what is added
    if (insert_len == len) {
        char save_char = str->buffer[pos+len];
        va_copy(args2, args);
        int m = vsnprintf(str->buffer+pos, len+1, fmt, args2);
        // TODO: check m
        va_end(args2);
        str->buffer[pos+len] = save_char;
        return 0;
    }

    int r = erase_and_insert_main_logic(arena, str, pos, len, insert_len);

    if (r != 0) {
        return -1;
    }

    char save_char = str->buffer[pos+insert_len];
    va_copy(args2, args);
    int m = vsnprintf(str->buffer+pos, insert_len+1, fmt, args2);
    // TODO check m
    va_end(args2);

    str->buffer[pos+insert_len] = save_char;
    str->size += (insert_len-len);
    return 0;
}

int string_erase_and_insert_fmt(Arena* arena, String* str, size_t pos, size_t len, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = string_erase_and_insert_vfmt(arena, str, pos, len, fmt, ap);
    va_end(ap);
    return r;
}

int string_clear(String* str) {
    if ((str == NULL) || (str->buffer == NULL)) return -1;
    str->size = 0;
    str->buffer[0] = '\0';
    return 0;
}

String string_deep_copy(Arena* arena, const String* str) {
    if ((str == NULL) || (str->buffer == NULL)) return (String){0};

    void* p = arena_alloc_push_struct(arena, (void*)str->buffer, str->capacity);

    if (p == NULL) return (String){0};

    return (String){
        .buffer = (char*)p,
        .size = str->size,
        .capacity = str->capacity,
    };
}

void string_debug_print(const String* s) {
    printf("String Debug Info:\n");
    printf(" String:      %p\n",             s);

    if (s == NULL) return;

    printf("  buffer:     %p\n",             s->buffer);
    printf("  size:       %lu bytes   %p\n", s->size, s->buffer+s->size+1);
    printf("  capacity:   %lu bytes   %p\n", s->capacity, s->buffer+s->capacity);
    printf("  free:       %lu bytes\n",      s->capacity - s->size - 1);
}

void string_debug_print(String s) {
    string_debug_print(&s);
}

void string_print(const String* s) {
    printf("%s", s->buffer);
}

//==============================
// STRING VIEW
//==============================
StringView sv_from_buffer(const char* buffer, size_t len) {
    if (buffer == NULL) {
        return (StringView){0};
    }
    return (StringView){
        .buffer = buffer,
        .size = len,
    };
}

StringView sv_from_cstr(const char* cstr) {
    if (cstr == NULL) {
        return (StringView){0};
    }
    return (StringView) {
        .buffer = cstr,
        .size = strlen(cstr),
    };
}

StringView sv_slice_sv(StringView sv, size_t start, size_t len) {
    if (start > sv.size) {
        return (StringView){0};
    }
    return (StringView) {
        .buffer = sv.buffer+start,
        .size = start+len < sv.size ? len : sv.size-start,
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
    size_t i = 0;
    for (; i<sv.size; ++i) {
        char c = sv.buffer[i];
        if (!((c >= 9 && c <= 13) || c == 32)) {
            break;
        }
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
    while (sv.size > 0) {
        char c = sv.buffer[sv.size-1];
        if (!((c>=9 && c<=13) || (c == 32))) {
            break;
        }
        sv.size--;
    }
    return sv;
}

bool sv_equal(StringView sv1, StringView sv2) {
    if (sv1.size != sv2.size) return false;
    int cmp = compare(sv1.buffer, sv2.buffer, sv1.size);
    return cmp == 0;
}

int sv_compare(const StringView* sv1, const StringView* sv2) {
    // -1 if sv1 < sv2
    // 0 if sv1 == sv2
    // 1 if sv1 > sv2
    size_t len = sv1->size<sv2->size ? sv1->size : sv2->size;

    int cmp = compare(sv1->buffer, sv2->buffer, len);
    if (cmp != 0) return cmp;

    if (sv1->size < sv2->size) return -1;
    if (sv1->size > sv2->size) return 1;
    return 0;
}

bool sv_starts_with(StringView sv, StringView pre) {
    if (sv.buffer == NULL || pre.buffer == NULL) {
        return false;
    }

    for (size_t i = 0; i<pre.size; ++i) {
        if (sv.buffer[i] != pre.buffer[i]) {
            return false;
        }
    }
    return true;
}

bool sv_ends_with(StringView sv, StringView pre) {
    if (sv.buffer == NULL || pre.buffer == NULL || sv.size < pre.size) {
        return false;
    }

    const char* buf = sv.buffer + sv.size-pre.size;
    for (size_t i = 0; i<pre.size; ++i) {
        if (buf[i] != pre.buffer[i]) {
            return false;
        }
    }
    return true;
}

size_t sv_find(StringView haystack, StringView needle) {
    if (haystack.buffer == NULL || needle.buffer == NULL) {
        return 0;
    }

    const unsigned char* s1 = (const unsigned char*) haystack.buffer;
    const unsigned char* s2 = (const unsigned char*) needle.buffer;

    size_t len = haystack.size-needle.size;
    size_t i = 0;
    for (size_t i = 0; i<len; i++, s1++) {
        if (compare(s1, s2, needle.size) == 0) {
            return i;
        }
    }

    return haystack.size;
}

size_t sv_rfind(StringView haystack, StringView needle) {
    if (haystack.buffer == NULL || needle.buffer == NULL) {
        return 0;
    }

    size_t len = haystack.size-needle.size;
    const unsigned char* s1 = (const unsigned char*) haystack.buffer + len;

    while(len >= 0) {
        if (compare(s1--, needle.buffer, needle.size) == 0) {
            return len;
        }
        len--;
    }

    return haystack.size;
}

void sv_print(StringView sv) {
    if (sv.buffer) {
        printf("%.*s", sv.size, sv.buffer);
    }
}

void sv_debug_print(StringView sv) {
    printf("StringView Debug Info:\n");
    printf("  buffer:     %p\n",             sv.buffer);
    printf("  size:       %lu bytes   %p\n", sv.size, sv.buffer+sv.size);
}
