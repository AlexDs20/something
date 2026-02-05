#include <stddef.h>
#include <stdarg.h>     // for variadic number of args.
#include <string.h>
#include <stdio.h>

#include "memory/allocators.h"
#include "libs/ads_string.h"

static size_t get_new_capacity(size_t new_size) {
    return new_size >= 16 ? (2*new_size)+1 : 16;
}

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

String string_init(Arena* arena, const char* init) {
    return string_init_from_buffer(arena, init, strlen(init));
}

String string_init_from_buffer(Arena* arena, const char* buffer, size_t len) {
    String str;

    // if (buffer == NULL) {
    //     return (String){0};
    // }

    str.size = len;
    str.capacity = get_new_capacity(str.size);

    str.buffer = (char*) arena_alloc_push(arena, str.capacity);
    if (str.buffer == NULL) {
        return (String){0};
    }
    if (buffer != NULL) {
        memcpy(str.buffer, buffer, len);
    }
    str.buffer[len] = '\0';

    return str;
}

String string_init_concat(Arena* arena, const char* first, const char* second) {
    String result;
    size_t l1;
    size_t l2;

    if ((first == NULL) || (second == NULL)) {
        return (String){0};
    }

    l1 = strlen(first);
    l2 = strlen(second);

    result = string_init_empty(arena, l1+l2+1);
    if (result.buffer == NULL) {
        return (String){0};
    }
    memcpy(result.buffer, first, l1);
    memcpy(result.buffer+l1, second, l2);

    result.size = l1+l2;
    result.buffer[result.size] = '\0';
    return result;
}

String string_init_fmt(Arena* arena, const char* fmt, ...) {
    String str;

    // Source: https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Variable-Number-of-Arguments.html
    va_list ap;

    // Sets ap to point before the first additional argument
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);        // Return the "written" size without '\0'
    // Fetch next additional argument
    // const char* v = va_arg(ap, const char*);
    va_end(ap);

    if (n < 0) {
        return (String){0};
    }

    size_t len = (size_t)n;

    str = string_init_from_buffer(arena, NULL, len);
    if (str.buffer == NULL) {
        return (String){0};
    }
    va_start(ap, fmt);
    vsnprintf(str.buffer, len+1, fmt, ap);
    va_end(ap);

    return str;
}

int string_append(Arena* arena, String* str, const char* post) {
    return string_append_len(arena, str, post, strlen(post));
}

int string_append_len(Arena* arena, String* str, const char* buffer, size_t len) {
    char* arena_top;
    char* string_end;
    size_t new_size;
    void* tmp;

    if ((str == NULL) || buffer == NULL) {
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

    memcpy(str->buffer+str->size, buffer, len);
    str->size = new_size;
    str->buffer[new_size] = '\0';
    return 0;
}

int string_append_char(Arena* arena, String* str, char c) {
    return string_append_len(arena, str, &c, 1);
}

int string_prepend(Arena* arena, String* str, const char* buffer) {
    return string_prepend_len(arena, str, buffer, strlen(buffer));
}

int string_prepend_len(Arena* arena, String* str, const char* buffer, size_t len) {
    size_t new_size;
    void* arena_top;
    void* tmp;

    if ((str == NULL) || (buffer == NULL)) {
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

    memcpy(str->buffer, buffer, len);
    str->size = new_size;
    return 0;
}

int string_prepend_char(Arena* arena, String* str, char c) {
    return string_prepend_len(arena, str, &c, 1);
}


void string_debug_print(String* s) {
    if ((s == NULL) || (s->buffer == NULL)) return;
    printf("String Debug Info:\n");
    printf("  buffer:     %p\n",             s->buffer);
    printf("  used:       %lu bytes   %p\n", s->size, s->buffer+s->size+1);
    printf("  capacity:   %lu bytes   %p\n", s->capacity, s->buffer+s->capacity);
    printf("  free:       %lu bytes\n",      s->capacity - s->size - 1);
}
