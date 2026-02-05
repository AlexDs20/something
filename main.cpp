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

#include "libs/ads_string.h"

#include "gf_profiling.c"


#if 0
int main() {
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
#else
int main() {
    Arena* arena = arena_alloc_create(64);
    arena_debug_print(arena);
    String s0 = string_init_empty(arena, 17);
    String s1 = string_init_cstr(arena, "Hello world s1");
    printf("S1: %s\n", s1.buffer);

    String s2 = string_init_buffer(arena, "Init from buffer", strlen("Init from buffer"));

    int s = string_append_buffer(arena, &s2, " !Append_buffer! " , 14);
    printf("%s\n", s2.buffer);
    s = string_append_cstr(arena, &s2, " !append! ");
    printf("%s\n", s2.buffer);

    s = string_append_char(arena, &s2, '=');
    printf("%s\n", s2.buffer);

    s = string_prepend_buffer(arena, &s2, "!prepend_buffer! ", strlen("!prepend_buffer! "));
    printf("%s\n", s2.buffer);

    s = string_prepend_cstr(arena, &s2, "!prepend! ");
    printf("%s\n", s2.buffer);

    s = string_prepend_char(arena, &s2, '?');
    printf("%s\n", s2.buffer);

    String s3 = string_init_fmt(arena, "This is my string_init_fmt %d %.3f %s", 3, 3.14, "PI");
    printf("%s\n", s3.buffer);
}
#endif
