#include <unistd.h>         // STDOUT_FILENO
#include <sys/syscall.h>    // syscall()
#include <stdio.h>

#include "platform/window.h"
#include "renderer/renderer.h"
#include "utils/allocators.h"

int main() {
    // syscalls: https://gpages.juszkiewicz.com.pl/syscalls-table/syscalls.html
    // 1 is write on x86_64
    const char msg[] = "Handmade something starts!\n";
    syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);

    char* file_path = "assets/backpack/backpack.obj";
    Model* model = read_model_file(file_path);

    uint16 w = 1024;
    uint16 h = 768;

    Vector* framebuffer = vector_alloc_create(w*h, sizeof(uint8));

    uint8 running = 1;
    while (running) {

        for (int i=0; i<vector_alloc_count(model->faces); ++i) {
            Face* f = (Face*)vector_alloc_get(model->faces, i);
            Vertex v[3] = {
                *((Vertex*)(model->vertices->buffer) + f->v[0]+1),
                *((Vertex*)(model->vertices->buffer) + f->v[1]+1),
                *((Vertex*)(model->vertices->buffer) + f->v[2]+1)
            };

            // Only use the x y components atm
            // Can work with perspective and camera later

            // v[0] => v[1]
            // v[1] => v[2]
            // v[2] => v[0]
            Vertex a = v[0];
            Vertex b = v[1];
            Vertex c = v[2];

            uint32 dx = a.x-b.x>0 ? a.x-b.x : -(a.x-b.x);
            uint32 dy = a.y-b.y>0 ? a.y-b.y : -(a.y-b.y);
            float t = 1/(float)(dx+dy);


            break;
        }

    }


    free_model(model);


    vector_alloc_free(framebuffer);

    // platform_main();
}
