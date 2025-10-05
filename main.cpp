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

    // string8 file_path = string_from_cstr(scene_arena, "assets/backpack/backpack.obj");
    // Model* model = {0};
    // if (string_get_file_extension(file_path) == ".obj") {
    //     model = read_obj_model_file(scene_arena, file_path);
    // }

    string8 file_path = string_from_cstr(scene_arena, "assets/backpack/diffuse.jpg");

    Image image = read_image_file(global_arena, file_path);

#if 1
    // u32* img_b = model->material->map_Kd.data;
    // u32  img_w = model->material->map_Kd.width;
    // u32  img_h = model->material->map_Kd.height;

    u32* img_b = image.buffer;
    u32  img_w = image.width;
    u32  img_h = image.height;
    const u32 w = img_w;
    const u32 h = img_h>512 ? 512 : img_h;
    // const u32 w = 4096;
    // const u32 h = 1024;

    const u32 bg_color = 0x777777;

    Win win = platform_init_win(w, h, msg);
    memcpy(win.buffer, img_b, w*h*4);
    // memcpy(win.buffer, model->material->map_Kd.data, w*h*4);
    // win.buffer = model->material->map_Kd.data;

    f32Bits zdefault = {.u = 0xFF7FFFFF};       // -Inf for IEEE 754 standard

    u32 running = 1;
    while (running) {
        arena_alloc_reset_zero(frame_arena);
        running = platform_handle_events(&win);

        // And this is uglier...
        // f32* zbuffer = (f32*)arena_alloc_push(frame_arena, win.h*win.w*sizeof(f32));

        // for (int i=0; i<win.h*win.w; i++) {
        //     win.buffer[i] = bg_color;
        //     zbuffer[i] = zdefault.f;
        // }

        // draw_model_wireframe(model, win.w, win.h, win.buffer);
        // draw_model(model, win.w, win.h, win.buffer, zbuffer);

        XPutImage(win.display, win.window, win.gc, win.xim, 0, 0, 0, 0, win.w, win.h);
        // usleep(16);
        // break;
    }
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
