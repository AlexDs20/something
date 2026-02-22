#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "memory/allocators.h"
#include "libs/ads_string.h"

static size_t get_new_capacity(size_t old_capacity, size_t new_size) {
    if (old_capacity > SIZE_MAX / 2) {
        return new_size + 1;
    }
    return 2*old_capacity > new_size+1 ? 2*old_capacity : new_size+1;
}

static int compare(const void* v1, const void* v2, size_t len) {
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

static int fmt_length(const char* fmt, va_list args) {
    int n;
    va_list args2;
    // Source: https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Variable-Number-of-Arguments.html
    va_copy(args2, args);
    n = vsnprintf(NULL, 0, fmt, args2);
    va_end(args2);
    return n;
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

    int n = fmt_length(fmt, args);
    if (n < 0) {
        return (String){0};
    }
    size_t len = (size_t)n;

    size_t capacity = get_new_capacity(0, len);

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

    str.capacity = get_new_capacity(0, sv.size);
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
    if (SIZE_MAX - str->size < append_len) {
        return -1;
    }

    size_t new_size = str->size + append_len;

    if (new_size + 1 > str->capacity) {
        void* arena_top = arena_alloc_used_location(arena);
        if (arena_top == NULL){
            return -1;
        }

        size_t new_capacity = get_new_capacity(str->capacity, new_size);

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
    str->size = new_size;
    str->buffer[str->size] = '\0';
    return 0;
}

int string_append_vfmt(Arena* arena, String* str, const char* fmt, va_list args){
    va_list args2;
    if (str == NULL || str->buffer == NULL || fmt == NULL) {
        return -1;
    }

    //==================================================
    // Source: https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Variable-Number-of-Arguments.html
    int n = fmt_length(fmt, args);
    if (n < 0) {
        return -1;
    }
    size_t len = (size_t)n;
    size_t old_size = str->size;

    int r = append_memory_logic(arena, str, len);
    if (r != 0) {
        return r;
    }

    va_copy(args2, args);
    int m = vsnprintf(str->buffer+old_size, len+1, fmt, args2);
    va_end(args2);
    if (m < 0 || (size_t) m != len) {
        return -1;
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
    if (SIZE_MAX - str->size < prepend_len) {
        return -1;
    }
    size_t new_size = str->size + prepend_len;

    if (new_size+1 > str->capacity) {
        size_t new_capacity = get_new_capacity(str->capacity, new_size);
        const char* arena_top = (char*)arena_alloc_used_location(arena);

        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL){
                return -1;
            }
            memmove(str->buffer+prepend_len, str->buffer, str->size+1);
        }
        else {
            char* t = (char*)arena_alloc_push(arena, new_capacity);
            if (t == NULL){
                return -1;
            }
            memcpy(t+prepend_len, str->buffer, str->size+1);
            str->buffer = t;
        }
        str->capacity = new_capacity;
    }
    else {
        memmove(str->buffer+prepend_len, str->buffer, str->size+1);
    }
    str->size = new_size;
    str->buffer[str->size] = '\0';
    return 0;
}

int string_prepend_vfmt(Arena* arena, String* str, const char* fmt, va_list args) {
    va_list args2;

    if (str == NULL || str->buffer == NULL || fmt == NULL) {
        return -1;
    }

    int n = fmt_length(fmt, args);
    if (n<0) {
        return -1;
    }
    size_t len = (size_t) n;

    const char save_char = str->buffer[0];

    int r = prepend_memory_logic(arena, str, len);
    if (r != 0) {
        return r;
    }

    va_copy(args2, args);
    int m = vsnprintf(str->buffer, len+1, fmt, args2);
    va_end(args2);
    if (m < 0 || (size_t) m != len) {
        return -1;
    }

    str->buffer[len] = save_char;
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
    if (SIZE_MAX - str->size < ins_len) {
        return -1;
    }
    size_t new_size = str->size + ins_len;

    if (new_size+1 > str->capacity) {
        size_t new_capacity = get_new_capacity(str->capacity, new_size);
        char* arena_top = (char*)arena_alloc_used_location(arena);

        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_unaligned(arena, new_capacity - str->capacity);
            if (t == NULL) {
                return -1;
            }
            memmove(str->buffer+pos+ins_len, str->buffer+pos, str->size+1 - pos);
        }
        else {
            char* t = (char*) arena_alloc_push(arena, new_capacity);
            if (t == NULL) {
                return -1;
            }
            memcpy(t, str->buffer, pos);
            memcpy(t+pos+ins_len, str->buffer+pos, str->size+1 - pos);
            str->buffer = t;
        }
        str->capacity = new_capacity;
    }
    else {
        memmove(str->buffer+pos+ins_len, str->buffer+pos, str->size+1 - pos);
    }

    str->size = new_size;
    str->buffer[str->size] = '\0';
    return 0;
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

    int n = vsnprintf(NULL, 0, fmt, args2);
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
    char save_char = str->buffer[pos+len];
    va_copy(args2, args);
    int m = vsnprintf(str->buffer+pos, len+1, fmt, args2);
    va_end(args2);
    if (m < 0 || (size_t) m != len) {
        return -1;
    }

    str->buffer[pos+len] = save_char;
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
    if (SIZE_MAX - pos < len) {
        return -1;
    }
    size_t new_size = pos+len > str->size ? pos+len : str->size;

    // Need to realloc
    if (new_size+1 > str->capacity) {
        char* arena_top = (char*)arena_alloc_used_location(arena);
        size_t new_capacity = get_new_capacity(str->capacity, new_size);

        if (arena_top == str->buffer+str->capacity) {
            void* t = arena_alloc_push_zero_unaligned(arena, new_capacity - str->capacity);
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
    }
    str->size = new_size;
    str->buffer[str->size] = '\0';
    return 0;
}

int string_overwrite_vfmt(Arena* arena, String* str, size_t pos, const char* fmt, va_list args) {
    va_list args2;

    if (str == NULL || str->buffer == NULL || pos > str->size || fmt == NULL) {
        return -1;
    }

    int n = fmt_length(fmt, args);
    if (n<0) {
        return -1;
    }
    size_t len = (size_t) n;

    int r = overwrite_memory_logic(arena, str, pos, len);
    if (r != 0) {
        return r;
    }

    char save_char = str->buffer[pos+len];
    va_copy(args2, args);
    int m = vsnprintf(str->buffer+pos, len+1, fmt, args2);
    va_end(args2);
    if (m < 0 || (size_t) m != len) {
        return -1;
    }
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

    memmove(str->buffer+pos, str->buffer+pos+len, str->size - (pos+len));
    str->size -= len;
    str->buffer[str->size] = '\0';
    return 0;
}

static int erase_and_insert_memory_logic(Arena* arena, String* str, size_t pos, size_t len, size_t insert_len) {
    if (insert_len > len && SIZE_MAX - str->size < insert_len - len) {
        return -1;
    }
    size_t new_size = str->size - len + insert_len;

    // If we grow beyond the current capacity => need more space
    if (new_size + 1 > str->capacity) {
        size_t new_capacity = get_new_capacity(str->capacity, new_size);
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
    str->size = new_size;
    str->buffer[str->size] = '\0';
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
        va_end(args2);
        if (m < 0 || (size_t) m != len) {
            return -1;
        }
        str->buffer[pos+len] = save_char;
        return 0;
    }

    int r = erase_and_insert_memory_logic(arena, str, pos, len, insert_len);

    if (r != 0) {
        return -1;
    }

    char save_char = str->buffer[pos+insert_len];
    va_copy(args2, args);
    int m = vsnprintf(str->buffer+pos, insert_len+1, fmt, args2);
    va_end(args2);
    if (m < 0 || (size_t) m != insert_len) {
        return -1;
    }

    str->buffer[pos+insert_len] = save_char;
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
    }
    else {
        printf("%s", s->buffer);
    }
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
    if ( (start >= sv.size) || (len > sv.size-start) ) {
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
    if (sv.buffer == NULL) return sv;
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
    if (sv.buffer == NULL) return sv;
    while (sv.size > 0) {
        unsigned char c = sv.buffer[sv.size-1];
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
    // <0 if sv1 < sv2
    // =0 if sv1 == sv2
    // >0 if sv1 > sv2
    size_t len = sv1->size<sv2->size ? sv1->size : sv2->size;

    int cmp = compare(sv1->buffer, sv2->buffer, len);
    if (cmp != 0) return cmp;

    if (sv1->size < sv2->size) return -1;
    if (sv1->size > sv2->size) return 1;
    return 0;
}

bool sv_starts_with(StringView sv, StringView pre) {
    if (sv.buffer == NULL || pre.buffer == NULL || pre.size > sv.size) {
        return false;
    }

    return compare(sv.buffer, pre.buffer, pre.size) == 0;
}

bool sv_ends_with(StringView sv, StringView post) {
    if (sv.buffer == NULL || post.buffer == NULL || sv.size < post.size) {
        return false;
    }

    return compare(sv.buffer + sv.size-post.size, post.buffer, post.size) == 0;
}

size_t sv_find(StringView haystack, StringView needle) {
    /*
     * Look into Boyer-Moore-Horspool for better perf.
     * https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm
     */
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

    size_t i = haystack.size-needle.size;

    while (true) {
        if ((h[i] == n[0]) && (memcmp(h+i , n, needle.size) == 0)) {
            return i;
        }
        if (i == 0) {
            break;
        }
        i--;
    }

    return haystack.size;
}

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
