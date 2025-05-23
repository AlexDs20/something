#include <unistd.h>         // STDOUT_FILENO
#include <sys/syscall.h>    // syscall()
#include <stdlib.h>
#include <string.h>

#include "platform/window.h"
#include "renderer/renderer.h"
#include "utils/defines.h"
#include "utils/io.h"

#include "gf_profiling.c"

typedef union {
    f32 f;
    u32 u;
} float_bits;

int main() {
    // syscalls: https://gpages.juszkiewicz.com.pl/syscalls-table/syscalls.html
    // 1 is write on x86_64
    char msg[] = "Handmade something starts!\n";
    syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);

    // main_io();

#if 1
    char file_path[] = "assets/backpack/backpack.obj";
    Model* model = read_model_file(file_path);

    const u32 w = 1024;
    const u32 h = 768;

    const u32 bg_color = 0x777777;

    Win win = platform_init_win(w, h, msg);

    f32* zbuffer = (f32*)malloc(win.max_h * win.max_w * sizeof(f32));
    float_bits zdefault = {.u = 0xFF7FFFFF};

    u32 running = 1;
    while (running) {
        running = platform_handle_events(&win);

        for (int i=0; i<win.h*win.w; i++) {
            u32* pixel = (u32*)win.buffer + i;
            *pixel = bg_color;
            f32* zpixel = (f32*)zbuffer + i;
            *zpixel = zdefault.f;
        }
        // draw_model_wireframe(model, win.w, win.h, win.buffer);
        draw_model(model, win.w, win.h, win.buffer, zbuffer);

        XPutImage(win.display, win.window, win.gc, win.xim, 0, 0, 0, 0, win.w, win.h);
        usleep(16);
    }

    free_model(model);
    free(win.buffer);
#endif

    char done_msg[] = "Done doing something!\n";
    syscall(1, STDOUT_FILENO, done_msg, sizeof(done_msg)-1);
}
