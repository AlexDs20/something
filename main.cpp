#include <unistd.h>         // STDOUT_FILENO
#include <sys/syscall.h>    // syscall()

#include "platform/window.h"
#include "renderer/renderer.h"

int main() {
    // syscalls: https://gpages.juszkiewicz.com.pl/syscalls-table/syscalls.html
    // 1 is write on x86_64
    const char msg[] = "Handmade something starts!\n";
    syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);

    char* file_path = "assets/backpack/backpack.obj";
    read_model_file(file_path);

    platform_main();
}
