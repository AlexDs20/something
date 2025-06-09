#include <stdio.h>
#include <string.h>
#include "utils/libstring.h"

string8 string_from_cstr(Arena* arena, const char* s) {
    string8 out = {};
    out.size = strlen(s);
    out.buffer = (u8*)arena_alloc_push(arena, out.size);
    if (out.buffer && out.size>0) {
        memcpy(out.buffer, s, out.size);
    }
    return out;
}

char* string_to_cstr(Arena* arena, string8 s) {
    char* cstr = (char*)arena_alloc_push(arena, s.size+1);
    if (cstr) {
        memcpy(cstr, s.buffer, s.size);
        cstr[s.size] = '\0';
    }
    return cstr;
}

void string_move_forward(string8* s, u64 amount){
    if (s->buffer && amount < s->size) {
        s->buffer += amount;
        s->size -= amount;
    }
}

string8 string_substring_after(string8 s, string8 pre) {
    string8 out = {0};
    if (string_starts_with(s, pre)) {
        out.buffer = s.buffer + pre.size;
        out.size = s.size - pre.size;
    }
    return out;
}

bool string_starts_with(string8 s, string8 pre) {
    if (pre.size > s.size) return false;

    for (u64 i=0; i<pre.size; i++) {
        if (pre.buffer[i] != s.buffer[i]) return false;
    }
    return true;
}

bool string_starts_with(string8 s, const char* pre) {
    u64 i=0;
    while (pre[i] != '\0') {
        if (pre[i] != s.buffer[i]) {
            return false;
        }
        i++;
    }
    return true;
}

bool string_ends_with(string8 s, string8 pre) {
    if (pre.size > s.size) return false;
    for (u64 i=0; i<pre.size; i++) {
        if (pre.buffer[pre.size-1-i] != s.buffer[s.size-1-i]) return false;
    }
    return true;
}

string8 string_copy_to_arena(Arena* arena, string8 string) {
    string8 out = {0};
    out.buffer = (u8*)arena_alloc_push(arena, string.size);
    if (out.buffer) {
        memcpy(out.buffer, string.buffer, string.size);
        out.size = string.size;
    }
    return out;
}

u64 string_find_first(string8 s, char c) {
    for (u64 i=0; i<s.size; i++) {
        if (s.buffer[i] == c) {
            return i;
        }
    }
    return s.size;
}

string8 string_substring(string8 s, u64 from, u64 to) {
    string8 out = {
        .buffer = &s.buffer[from],
        .size = to - from,
    };
    return out;
}

string8 string_dirname(string8 s) {
    for (u64 i=0; i<s.size; i++) {
        if (s.buffer[s.size-1 - i] == '/') {
            string8 out = {
                .buffer = s.buffer,
                .size = s.size - i,
            };
            return out;
        }
    }
    return s;
}

string8 string_join_strings(Arena* arena, string8 a, string8 b) {
    string8 out = {
        .buffer = 0,
        .size = a.size + b.size,
    };
    out.buffer = (u8*)arena_alloc_push(arena, out.size);
    if (out.buffer) {
        memcpy(out.buffer, a.buffer, a.size);
        memcpy(&(out.buffer[a.size]), b.buffer, b.size);
    }
    return out;
}

string8 string_get_file_extension(string8 s) {
    string8 out = {0};
    for (u64 i=0; i<s.size; i++) {
        if (s.buffer[s.size-1 - i] == '.') {
            out.buffer = &s.buffer[s.size-1 - i];
            out.size = i+1;
        }
    }
    return out;
}

bool operator==(string8 a, string8 b) {
    if (!a.buffer || !b.buffer || a.size != b.size) {
        return false;
    }
    for (u64 i=0; i<a.size; i++) {
        if (a.buffer[i] != b.buffer[i]) {
            return false;
        }
    }
    return true;
}

bool operator==(string8 a, const char* c) {
    if (!a.buffer) {
        return false;
    }
    for (u64 i=0; i<a.size; i++) {
        if ((a.buffer[i] != c[i]) || (c[i] == '\0')) {
            return false;
        }
    }
    return c[a.size] == '\0';
}

void string_print(string8 s) {
    for (u64 i = 0; i < s.size; ++i) {
        printf("%c", (char)s.buffer[i]);
    }
}
