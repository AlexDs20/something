#include <stddef.h>
#include <stdarg.h>     // for variadic number of args.
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
    String str;

    if (capacity <= 0) {
        return (String){0};
    }

    str.buffer = (char*) arena_alloc_push_zero(arena, capacity);

    if (str.buffer == NULL) {
        return (String){0};
    }

    str.size = 0;
    str.capacity = capacity;
    str.buffer[0] = '\0';
    return str;
}

String string_init_cstr(Arena* arena, const char* init) {
    if (init == NULL) {
        return (String){0};
    }
    return string_init_buffer(arena, init, strlen(init));
}

String string_init_buffer(Arena* arena, const char* buffer, size_t len) {
    String str;

    str.capacity = get_new_capacity(len);
    str.size = buffer == NULL ? 0 : len;

    str.buffer = (char*) arena_alloc_push(arena, str.capacity);
    if (str.buffer == NULL) {
        return (String){0};
    }
    if (buffer != NULL) {
        memcpy(str.buffer, buffer, len);
    }
    if (buffer) {
        str.buffer[len] = '\0';
    }
    else if (str.capacity >= 0){
        str.buffer[0] = '\0';
    }

    return str;
}

String string_init_concat(Arena* arena, const char* first, const char* second) {
    String result;
    size_t l1;
    size_t l2;

    if ((first == NULL) && (second == NULL)) {
        return (String){0};
    }

    l1 = 0;
    if (first) {
        l1 = strlen(first);
    }
    l2 = 0;
    if (second) {
        l2 = strlen(second);
    }

    result = string_init_empty(arena, l1+l2+1);
    if (result.buffer == NULL) {
        return (String){0};
    }
    if (first) {
        memcpy(result.buffer, first, l1);
    }
    if (second) {
        memcpy(result.buffer+l1, second, l2);
    }

    result.size = l1+l2;
    result.buffer[result.size] = '\0';
    return result;
}

String string_init_fmt(Arena* arena, const char* fmt, ...) {
    if (fmt == NULL) {
        return (String){0};
    }
    String str;

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

    str = string_init_buffer(arena, NULL, len);
    str.size = len;
    if (str.buffer == NULL) {
        return (String){0};
    }
    va_start(ap, fmt);
    vsnprintf(str.buffer, len+1, fmt, ap);
    va_end(ap);

    return str;
}

String string_init_sv(Arena* arena, StringView sv) {
    return string_init_buffer(arena, sv.buffer, sv.size);
}

int string_append_buffer(Arena* arena, String* str, const char* buffer, size_t len) {
    char* arena_top;
    char* string_end;
    size_t new_size;
    void* tmp;

    // if ((str == NULL) || buffer == NULL) {
    if (str == NULL) {
        return -1;
    }

    new_size = str->size + len;

    // Need to "realloc"
    if (new_size + 1 > str->capacity) {
        size_t new_capacity = get_new_capacity(new_size);

        arena_top = (char*)arena_alloc_used_location(arena);
        if (arena_top == NULL) {
            return -1;
        }

        // If our current string ends at the top of arena -> we are gucci!
        // We just push onto the arena
        string_end = str->buffer+str->capacity;
        if (arena_top == string_end) {
            size_t bytes_needed = (char*)(str->buffer + new_capacity) - (char*)arena_top;
            tmp = arena_alloc_push_zero_unaligned(arena, bytes_needed);
            if (tmp == NULL) {
                return -1;
            }
        }
        else {
            tmp = arena_alloc_push(arena, new_capacity);
            if (tmp == NULL) {
                return -1;
            }
            if (str->buffer) {
                memcpy(tmp, str->buffer, str->size);
            }
            str->buffer = (char*)tmp;
        }
        str->capacity = new_capacity;
    }

    if (buffer) {
        memcpy(str->buffer+str->size, buffer, len);
    }
    str->size = new_size;
    str->buffer[new_size] = '\0';
    return 0;
}

int string_append_cstr(Arena* arena, String* str, const char* post) {
    return string_append_buffer(arena, str, post, strlen(post));
}

int string_append_string(Arena* arena, String* str, const String* append) {
    return string_append_buffer(arena, str, append->buffer, append->size);
}

int string_append_sv(Arena* arena, String* str, StringView append) {
    return string_append_buffer(arena, str, append.buffer, append.size);
}

int string_append_char(Arena* arena, String* str, char c) {
    return string_append_buffer(arena, str, &c, 1);
}

int string_append_fmt(Arena* arena, String* str, const char* fmt, ...) {
    // Source: https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Variable-Number-of-Arguments.html
    va_list ap;

    // Sets ap to point before the first additional argument
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);        // Return the "written" size without '\0'
    va_end(ap);

    if (n < 0) {
        return -1;
    }

    size_t len = (size_t)n;

    int fail = string_append_buffer(arena, str, NULL, len);
    if (fail) {
        return fail;
    }

    va_start(ap, fmt);
    vsnprintf(str->buffer, len+1, fmt, ap);
    va_end(ap);
    return 0;
}

int string_prepend_buffer(Arena* arena, String* str, const char* buffer, size_t len) {
    size_t new_size;
    void* arena_top;
    void* tmp;

    if (str == NULL) {
        return -1;
    }

    new_size = str->size + len;

    // Need to realloc
    if (new_size + 1 > str->capacity) {
        size_t new_capacity = get_new_capacity(new_size);

        arena_top = arena_alloc_used_location(arena);
        if (arena_top == NULL) {
            return -1;
        }

        void* string_end = str->buffer + str->capacity;
        // If our string is already the last one in the arena, then we can keep on working on this one
        if (arena_top == string_end) {
            size_t bytes_needed = (char*)(str->buffer + new_capacity) - (char*)arena_top;
            tmp = arena_alloc_push_zero_unaligned(arena, bytes_needed);
            if (tmp == NULL) {
                return -1;
            }
            memmove(str->buffer+len, str->buffer, str->size+1);
        }
        else {
            tmp = arena_alloc_push(arena, new_capacity);
            if (tmp == NULL) {
                return -1;
            }
            memcpy((char*)tmp+len, str->buffer, str->size+1);
            str->buffer = (char*)tmp;
        }
        str->capacity = new_capacity;
    }
    else {
        memmove(str->buffer+len, str->buffer, str->size+1);
    }

    if (buffer) {
        memcpy(str->buffer, buffer, len);
    }
    str->size = new_size;
    return 0;
}

int string_prepend_cstr(Arena* arena, String* str, const char* buffer) {
    return string_prepend_buffer(arena, str, buffer, strlen(buffer));
}

int string_prepend_string(Arena* arena, String* str, const String* pre) {
    return string_prepend_buffer(arena, str, pre->buffer, pre->size);
}

int string_prepend_sv(Arena* arena, String* str, StringView pre) {
    return string_prepend_buffer(arena, str, pre.buffer, pre.size);
}

int string_prepend_char(Arena* arena, String* str, char c) {
    return string_prepend_buffer(arena, str, &c, 1);
}

int string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (n<0) {
        return -1;
    }

    size_t len = (size_t) n;
    int fail = string_prepend_buffer(arena, str, NULL, len);
    if (fail) {
        return fail;
    }

    const char save_char = str->buffer[len];
    va_start(ap, fmt);
    vsnprintf(str->buffer, len+1, fmt, ap);
    str->buffer[len] = save_char;
    va_end(ap);
    return 0;
}

int string_insert_buffer(Arena* arena, String* str, size_t pos, const char* buffer, size_t len) {
    if ((str == NULL) || (str->buffer == NULL)) {
        return -1;
    }
    if (pos > str->size) {
        return -1;
    }

    size_t new_size = str->size + len;

    // If enough space
    if (new_size < str->capacity) {
        memmove(str->buffer+pos+len, str->buffer+pos, str->size - pos+1);
        if (buffer) {
            memcpy(str->buffer+pos, buffer, len);
        }
        else {
            memset(str->buffer+pos, ' ', len);
        }
        str->buffer[new_size] = '\0';
        str->size = new_size;
        return 0;
    }
    size_t new_capacity = get_new_capacity(new_size);
    // If not enough space
    //  And not at end of arena => Need to copy everything
    char* arena_top = (char*)arena_alloc_used_location(arena);
    if (str->buffer + str->capacity != arena_top) {
        char* p = (char*)arena_alloc_push(arena, new_capacity);
        if (p == NULL) return -1;

        memcpy(p, str->buffer, pos);
        if (buffer) {
            memcpy(p+pos, buffer, len);
        }
        else {
            memset(p+pos, ' ', len);
        }
        memcpy(p+pos+len, str->buffer+pos, str->size - pos);
        str->buffer = p;
        str->buffer[new_size] = '\0';
        str->size = new_size;
        str->capacity = new_capacity;
        return 0;
    }
    // If not enough space
    //  and at the end of the arena
    // Do "in place" insert. Only need to move what's after the position
    void* p = arena_alloc_push(arena, str->buffer+new_capacity - arena_top);
    if (p == NULL) return -1;
    memmove(str->buffer+pos+len, str->buffer+pos, str->size - pos);
    if (buffer) {
        memcpy(str->buffer+pos, buffer, len);
    }
    else {
        memset(str->buffer+pos, ' ', len);
    }
    str->buffer[new_size] = '\0';
    str->size = new_size;
    str->capacity = new_capacity;
    return 0;
}

int string_insert_cstr(Arena* arena, String* str, size_t pos, const char* cstr) {
    if (cstr == NULL) return -1;
    return string_insert_buffer(arena, str, pos, cstr, strlen(cstr));
}

int string_insert_string(Arena* arena, String* str, size_t pos, const String* ins) {
    if ((ins == NULL) || (ins->buffer)) {
        return -1;
    }
    return string_insert_buffer(arena, str, pos, ins->buffer, ins->size);
}

int string_insert_sv(Arena* arena, String* str, size_t pos, StringView ins) {
    if (ins.buffer) {
        return -1;
    }
    return string_insert_buffer(arena, str, pos, ins.buffer, ins.size);
}

int string_insert_char(Arena* arena, String* str, size_t pos, char c) {
    return string_insert_buffer(arena, str, pos, &c, 1);
}

int string_insert_fmt(Arena* arena, String* str, size_t pos, const char* fmt, ...){
    va_list ap;

    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (n<0) {
        return -1;
    }

    size_t len = (size_t) n;

    int fail = string_insert_buffer(arena, str, pos, NULL, len);
    if (fail) {
        return fail;
    }

    // Save first char of what is after the insert because vsnprintf always puts a \0 ...
    char save_char = str->buffer[pos+len];

    va_start(ap, fmt);
    vsnprintf(str->buffer+pos, len+1, fmt, ap);
    str->buffer[pos+len] = save_char;
    va_end(ap);
    return 0;
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
    printf("%s\n", s->buffer);
}

//==============================
// STRING VIEW
//==============================
StringView string_to_sv(String str) {
    return (StringView){
        .buffer = str.buffer,
        .size = str.size,
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

StringView sv_from_string(String str) {
    return string_to_sv(str);
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
