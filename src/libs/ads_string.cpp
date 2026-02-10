#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "memory/allocators.h"
#include "libs/ads_string.h"

static size_t get_new_capacity(size_t new_size) {
    return new_size >= 16 ? (2*new_size)+1 : 16;
}

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
    String str = {0};
    if (fmt == NULL) {
        return str;
    }

    // Source: https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Variable-Number-of-Arguments.html
    va_list ap;

    // Sets ap to point before the first additional argument
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);        // Return the "written" size without '\0'
    va_end(ap);

    if (n < 0) {
        return (String){0};
    }

    size_t len = (size_t)n;

    size_t capacity = get_new_capacity(len);
    str.buffer = (char*) arena_alloc_push(arena, capacity);
    if (str.buffer == NULL) {
        return (String){0};
    }
    va_start(ap, fmt);
    vsnprintf(str.buffer, len+1, fmt, ap);          // Write len+1 elements including '\0'
    va_end(ap);

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
    return str->buffer;
}

int string_append_fmt(Arena* arena, String* str, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = string_append_vfmt(arena, str, fmt, ap);
    va_end(ap);
    return r;
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

    if (str->size+1+len > str->capacity) {
        int fail = string_grow_capacity(arena, str, len);
        if (fail) {
            return fail;
        }
    }

    va_copy(args2, args);
    vsnprintf(str->buffer+str->size, len+1, fmt, args2);
    va_end(args2);
    str->size += len;
    //==================================================

    return 0;
}

// TODO(alex): Check whether the StringView overlaps the str! in that case use memmove and not memcopy
int string_append_sv(Arena* arena, String* str, StringView append) {
    char* arena_top;
    size_t new_size;
    void* tmp;

    if (str == NULL || str->buffer == NULL) {
        return -1;
    }
    if (append.buffer == NULL){
        return -1;
    }

    new_size = str->size + append.size;

    // Need to "realloc"
    if (new_size + 1 > str->capacity) {
        size_t new_capacity = get_new_capacity(new_size);

        arena_top = (char*)arena_alloc_used_location(arena);
        if (arena_top == NULL) {
            return -1;
        }

        // If our current string ends at the top of arena -> we are gucci!
        // We just push onto the arena
        if (arena_top == str->buffer+str->capacity) {
            size_t bytes_needed = new_capacity - str->capacity;
            tmp = arena_alloc_push_unaligned(arena, bytes_needed);
            if (tmp == NULL) {
                return -1;
            }
        }
        else {
            tmp = arena_alloc_push(arena, new_capacity);
            if (tmp == NULL) {
                return -1;
            }
            memcpy(tmp, str->buffer, str->size);
            str->buffer = (char*)tmp;
        }
        str->capacity = new_capacity;
    }
    else

    memcpy(str->buffer+str->size, append.buffer, append.size);
    str->size = new_size;
    str->buffer[new_size] = '\0';
    return 0;
}

int string_grow_capacity(Arena* arena, String* str, size_t amount) {
    if (str == NULL || str->buffer == NULL) {
        return -1;
    }

    char* arena_top = (char*)arena_alloc_used_location(arena);
    if (arena_top == NULL) {
        return -1;
    }
    if (arena_top == str->buffer + str->capacity) {
        void* tmp = arena_alloc_push_unaligned(arena, amount);
        if (tmp == NULL) return -1;
    }
    else {
        void* tmp = arena_alloc_push(arena, str->capacity+amount);
        if (tmp == NULL) return -1;

        memcpy(tmp, str->buffer, str->size+1);

        str->buffer = (char*)tmp;
    }
    str->capacity += amount;

    return 0;
}

int string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = string_prepend_vfmt(arena, str, fmt, ap);
    va_end(ap);
    return r;
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

// TODO(alex): Handle if the StringView is in the current string!!
int string_prepend_sv(Arena* arena, String* str, StringView pre) {
    size_t new_size;
    char* arena_top;
    char* tmp;

    if ((str == NULL) || (str->buffer == NULL) || (pre.buffer == NULL)) {
        return -1;
    }

    new_size = str->size + pre.size;

    // Need to realloc
    if (new_size + 1 > str->capacity) {
        size_t new_capacity = get_new_capacity(new_size);

        arena_top = (char*)arena_alloc_used_location(arena);
        if (arena_top == NULL) {
            return -1;
        }

        if (arena_top == str->buffer + str->capacity) {
            size_t bytes_needed = new_capacity - str->capacity;
            tmp = (char*)arena_alloc_push_unaligned(arena, bytes_needed);
            if (tmp == NULL) {
                return -1;
            }
            memmove(str->buffer+pre.size, str->buffer, str->size+1);
        }
        else {
            tmp = (char*)arena_alloc_push(arena, new_capacity);
            if (tmp == NULL) {
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

    memcpy(str->buffer, pre.buffer, pre.size);
    str->size = new_size;
    return 0;
}

int string_insert_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...){
    va_list ap;

    if (str == NULL || str->buffer == NULL || fmt == NULL || pos > str->size) {
        return -1;
    }

    va_start(ap, fmt);

    if (pos == 0) {
        int r = string_prepend_vfmt(arena, str, fmt, ap);
        va_end(ap);
        return r;
    } else if (pos == str->size) {
        int r = string_append_vfmt(arena, str, fmt, ap);
        va_end(ap);
        return r;
    }

    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

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


    va_start(ap, fmt);
    vsnprintf(str->buffer+pos, len+1, fmt, ap);
    va_end(ap);
    str->buffer[pos+len] = save_char;
    str->size += len;
    return 0;
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

int string_overwrite_fmt(String* str, size_t pos, const char* fmt, ...) {
    va_list ap;

    if (str == NULL || str->buffer == NULL || fmt == NULL) {
        return -1;
    }

    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (n<0) return -1;

    size_t len = (size_t) n;

    size_t end = pos+len > str->size ? str->size : pos+len;

    char save_char = str->buffer[end];

    va_start(ap, fmt);
    vsnprintf(str->buffer+pos, end+1-pos, fmt, ap);
    va_end(ap);
    str->buffer[end] = save_char;
    return 0;
}

int string_overwrite_sv(String* str, size_t pos, StringView sv) {
    if (str == NULL || str->buffer == NULL || pos > str->size || sv.buffer == NULL) {
        return -1;
    }

    // NOTE: This allows to have a sv too big but erase_and_insert_sv does not
    size_t end = pos+sv.size > str->size ? str->size : pos+sv.size;

    memcpy(str->buffer+pos, sv.buffer, end - pos);

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
    int size_difference = len - sv.size;
    if (size_difference == 0) {
        memcpy(str->buffer + pos, sv.buffer, sv.size);
        return 0;
    }

    size_t new_size = str->size + size_difference;

    // If we grow beyond the current capacity => need more space
    if (str->size+1+size_difference > str->capacity) {
        size_t new_capacity = get_new_capacity(str->size+size_difference);
        char* arena_top = (char*)arena_alloc_used_location(arena);
        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) return -1;
            memmove(str->buffer+pos+sv.size, str->buffer+pos+len, str->size+1 - (pos+len));
        }
        else {
            char* t = (char*)arena_alloc_push(arena, new_capacity);
            if (t == NULL) return -1;
            memcpy(t, str->buffer, pos);
            memcpy(t+pos+sv.size, str->buffer+pos+len, str->size+1-(pos+len));
            str->buffer = t;
        }
        str->capacity = new_capacity;
    }
    else {
        memmove(str->buffer+pos+sv.size, str->buffer+pos+len, str->size+1 - (pos+len));
    }

    memcpy(str->buffer+pos, sv.buffer, sv.size);
    str->size = new_size;
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

StringView sv_from_string(String str) {
    return (StringView){
        .buffer = str.buffer,
        .size = str.size,
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

StringView sv_slice_string(String str, size_t start, size_t len) {
    if (start > str.size) {
        return (StringView){0};
    }
    return (StringView){
        .buffer = str.buffer + start,
        .size = start+len < str.size ? len : str.size-start,
    };
}

StringView sv_slice_buffer(const char* buffer, size_t start, size_t len) {
    if (buffer == NULL) {
        return (StringView){0};
    }
    return (StringView) {
        .buffer = buffer+start,
        .size = start+len < strlen(buffer) ? len : strlen(buffer)-start,
    };
}

StringView sv_slice(StringView sv, size_t start, size_t len) {
    if (start > sv.size) {
        return (StringView){0};
    }
    return (StringView) {
        .buffer = sv.buffer+start,
        .size = start+len < sv.size ? len : sv.size-start,
    };
}

void sv_print(StringView sv) {
    if (sv.buffer) {
        printf("%.*s\n", sv.size, sv.buffer);
    }
}

void sv_debug_print(StringView sv) {
    printf("StringView Debug Info:\n");
    printf("  buffer:     %p\n",             sv.buffer);
    printf("  size:       %lu bytes   %p\n", sv.size, sv.buffer+sv.size);
}
