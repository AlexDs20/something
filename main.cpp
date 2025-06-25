#include <unistd.h>         // STDOUT_FILENO
#include <sys/syscall.h>    // syscall()
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "memory/allocators.h"
#include "platform/window.h"
#include "renderer/renderer.h"
#include "libs/libstring.h"
#define LIB_IMAGES_IMPLEMENTATION
#include "libs/libimages.h"
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

#if 1
    string8 file_path = string_from_cstr(scene_arena, "assets/backpack/backpack.obj");
    Model* model = read_obj_model_file(scene_arena, file_path);

    const u32 w = 1024;
    const u32 h = 768;

    const u32 bg_color = 0x777777;

    Win win = platform_init_win(w, h, msg);

    f32Bits zdefault = {.u = 0xFF7FFFFF};       // -Inf for IEEE 754 standard

    u32 running = 1;
    while (running) {
        arena_alloc_reset_zero(frame_arena);
        running = platform_handle_events(&win);

        // And this is uglier...
        f32* zbuffer = (f32*)arena_alloc_push(frame_arena, win.h*win.w*sizeof(f32));

        for (int i=0; i<win.h*win.w; i++) {
            win.buffer[i] = bg_color;
            zbuffer[i] = zdefault.f;
        }

        // draw_model_wireframe(model, win.w, win.h, win.buffer);
        draw_model(model, win.w, win.h, win.buffer, zbuffer);

        XPutImage(win.display, win.window, win.gc, win.xim, 0, 0, 0, 0, win.w, win.h);
        usleep(16);
        break;
    }
    string8 outfile = string_from_cstr(frame_arena, "test.jpeg");
    write_image_file(win.buffer, win.w, win.h, outfile);
#endif

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
