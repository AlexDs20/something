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

int main() {
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

    u32* data = model->material->map_Kd.data;
    u32 img_w = model->material->map_Kd.width;
    u32 img_h = model->material->map_Kd.height;

    const u32 window_w = 640;
    const u32 window_h = 480;
    const u32 bg_color = 0x777777;
    // TODO: Add support for RGB and GREY currently only RGBA
    Win win = platform_init_win(window_w, window_h, msg);

    const f32Bits zdefault = {.u = 0xFF7FFFFF};       // -Inf for IEEE 754 standard

    u32 running = 1;
    while (running) {
        arena_alloc_reset_zero(frame_arena);
        running = platform_handle_events(&win);

        // And this is uglier...
        f32* zbuffer = (f32*)arena_alloc_push(frame_arena, win.h*win.w*sizeof(f32));
        u32* win_buffer = (u32*)arena_alloc_push(frame_arena, img_w*img_h*sizeof(f32));
        // DATA format: [RR] [GG] [BB] [AA]
        // on little-endian: 0xAABBGGRR
        // on big-endian: 0xRRGGBBAA

        // for (int i=0; i<win.h*win.w; i++) {
        //     win_buffer[i] = bg_color;
        //     zbuffer[i] = zdefault.f;
        // }

        // draw_model_wireframe(model, win.w, win.h, win_buffer);
        // draw_model(model, win.w, win.h, win_buffer, zbuffer);

        memcpy(win_buffer, data, img_w*img_h*sizeof(u32));

        // platform_render_to_window((u8*)win_buffer, w, h, 4, &win);
        platform_render_to_window((u8*)win_buffer, img_w, img_h, 4, &win);
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
