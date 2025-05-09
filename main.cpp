#include <unistd.h>         // STDOUT_FILENO
#include <sys/syscall.h>    // syscall()
#include <stdlib.h>
#include <string.h>

#include "platform/window.h"
#include "renderer/renderer.h"
#include "utils/defines.h"


int main() {
    // syscalls: https://gpages.juszkiewicz.com.pl/syscalls-table/syscalls.html
    // 1 is write on x86_64
    const char msg[] = "Handmade something starts!\n";
    syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);

    char* file_path = "assets/backpack/backpack.obj";
    Model* model = read_model_file(file_path);

    const uint32 w = 1024;
    const uint32 h = 768;

    const uint32 bg_color = 0x00FF0000;

    uint32* framebuffer = (uint32*)malloc(w*h*sizeof(uint32));
    memset(framebuffer, bg_color, w*h*sizeof(uint32));

    uint32 running = 1;
    while (running) {
        draw_model_wireframe(model, w, h, framebuffer);
        break;
    }

    // platform_main();

    free_model(model);
    free(framebuffer);

    char* done_msg = "DONE!\n";
    syscall(1, STDOUT_FILENO, done_msg, sizeof(done_msg)-1);
}
