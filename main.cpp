#include <unistd.h>         // STDOUT_FILENO
#include <sys/syscall.h>    // syscall()
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "memory/allocators.h"
#include "platform/window.h"
#include "renderer/renderer.h"
#include "libs/libstring.h"
#include "libs/ads_images.h"
#include "utils/types.h"
#include "utils/defines.h"
#include "utils/io.h"
#include "platform/memory.h"

#include "gf_profiling.c"


int main_main() {
    //
    // syscalls: https://gpages.juszkiewicz.com.pl/syscalls-table/syscalls.html
    // 1 is write on x86_64
    char msg[] = "Handmade something starts!\n";
    syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);

    Arena* global_arena = arena_alloc_create(1*GiB);
    Arena* scene_arena = arena_alloc_create(1*GiB);
    Arena* frame_arena = arena_alloc_create(1*GiB);

    string8 file_path = string_from_cstr(scene_arena, "assets/backpack/backpack.obj");
    Model* model = {0};
    if (string_get_file_extension(file_path) == ".obj") {
        model = read_obj_model_file(scene_arena, file_path);
    }

    // u32* win_buffer = model->material->map_Kd.buffer;
    // u32 canvas_w = model->material->map_Kd.width;
    // u32 canvas_h = model->material->map_Kd.height;

    u32 canvas_w = 1920;
    u32 canvas_h = 1080;
    const u32 bg_color = 0x18181B;
    // TODO: Add support for RGB and GREY currently only RGBA
    Win win = platform_init_win(1920, 1080, "Handmade something!", ADSV_NEAREST);

    const f32Bits zdefault = {.u = 0xFF7FFFFF};       // -Inf for IEEE 754 standard

    u32 running = 1;

    // ColorContext frag_context = {
    //     0xFF777777,
    // };

    TextureContext frag_context = {
        .texture = &model->material->map_Kd,
    };

    while (running) {
        arena_alloc_reset_zero(frame_arena);
        running = platform_handle_events(&win);

        // Allocate the canvas size we want to draw onto
        // It does not have to be the same size as the window
        canvas_w = win.w;
        canvas_h = win.h;

        f32* zbuffer = (f32*)arena_alloc_push(frame_arena, canvas_w*canvas_h*sizeof(f32));
        u32* win_buffer = (u32*)arena_alloc_push(frame_arena, canvas_w*canvas_h*sizeof(f32));

        // DATA format: [RR] [GG] [BB] [AA]
        // on little-endian: 0xAABBGGRR
        // on big-endian: 0xRRGGBBAA

        for (int i=0; i<canvas_h*canvas_w; i++) {
            win_buffer[i] = bg_color;
            zbuffer[i] = zdefault.f;
        }

        // draw_model_wireframe(model, canvas_w, canvas_h, win_buffer);
        draw_model(model, canvas_w, canvas_h, win_buffer, zbuffer, (void*) (&frag_context), shader_frag_texture);
        // draw_model(model, canvas_w, canvas_h, win_buffer, zbuffer, (void*) (&frag_context), shader_frag_depth);
        // draw_model(model, canvas_w, canvas_h, win_buffer, zbuffer, (void*) (&frag_context), shader_frag_color);

        platform_render_to_window((u8*)win_buffer, canvas_w, canvas_h, &win);
    }

    char done_msg[] = "Done doing something!\n";
    syscall(1, STDOUT_FILENO, done_msg, sizeof(done_msg)-1);

    // No need
    if (true) {
        platform_cleanup_window(win);
        arena_alloc_free(global_arena);
        arena_alloc_free(frame_arena);
        arena_alloc_free(scene_arena);
    }
}


#include <stddef.h>

typedef struct {
    char* buffer;
    size_t size;
    size_t capacity;
} String;

String string_init_empty(Arena* arena, size_t capacity);
String string_init(Arena* arena, const char* init);
String string_init_concat(Arena* arena, const char* first, const char* second);
// String string_init_fmt(Arena* arena, const char* fmt, ...);
String string_init_from_buffer(Arena* arena, const char* buffer, size_t len);
// int    string_append(Arena* arena, String* str, const char* post);
int    string_append_len(Arena* arena, String* str, const char* buffer, size_t len);
// int    string_append_char(Arena* arena, String* str, char c);
// int    string_append_fmt(Arena* arena, String* str, const char* fmt, ...);
// int    string_prepend(Arena* arena, String* str, const char* pre);
// int    string_prepend_char(Arena* arena, String* str, char c);
// int    string_prepend_fmt(Arena* arena, String* str, const char* fmt, ...);
void   string_debug_print(String* string);


int main() {
    Arena* arena = arena_alloc_create(64);
    arena_debug_print(arena);
    // String s0 = string_init_empty(arena, 17);
    // arena_debug_print(arena);
    // string_debug_print(&s0);
    // String s1 = string_init(arena, "Hellow there");
    // printf("Size: %d\n", strlen("Hellow there"));
    // arena_debug_print(arena);
    // string_debug_print(&s1);
    String s2 = string_init_from_buffer(arena, "Hellow I am there", 17);
    printf("Size: %d\n", 17);
    string_debug_print(&s2);
    arena_debug_print(arena);
    int s = string_append_len(arena, &s2, "This is me agaaaain there " , 26);
    printf("Size: %d\n", 26);
    string_debug_print(&s2);
    arena_debug_print(arena);
    printf("%s\n", s2.buffer);
}

#include <string.h>
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

    if (buffer == NULL) {
        return (String){0};
    }

    str.size = len;
    str.capacity = get_new_capacity(str.size); // str.size >= 16 ? (2*str.size)+1 : 16;

    str.buffer = (char*) arena_alloc_push(arena, str.capacity);
    if (str.buffer == NULL) {
        return (String){0};
    }
    memcpy(str.buffer, buffer, len);
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

// String string_init_from_fmt(Arena* const char* fmt, ...) {
// }

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
        size_t new_capacity = get_new_capacity(new_size); // new_size >= 16 ? (2*new_size)+1 : 16;

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

    memcpy(str->buffer+str->size, buffer, strlen(buffer) >= len ? len : strlen(buffer));
    str->size = new_size;
    str->buffer[new_size] = '\0';
    return 0;
}


// int string_append(Arena* arena, String* str, const char* post) {
//     // Check If this str has the capacity to append
//     // If not => need to "realloc"
//     // And append
// }

void string_debug_print(String* s) {
    if ((s == NULL) || (s->buffer == NULL)) return;
    printf("String Debug Info:\n");
    printf("  buffer:     %p\n",             s->buffer);
    printf("  used:       %lu bytes   %p\n", s->size, s->buffer+s->size+1);
    printf("  capacity:   %lu bytes   %p\n", s->capacity, s->buffer+s->capacity);
    printf("  free:       %lu bytes\n",      s->capacity - s->size - 1);
}
