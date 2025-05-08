#include <unistd.h>         // STDOUT_FILENO
#include <sys/syscall.h>    // syscall()
#include <stdio.h>

#include "platform/window.h"
#include "renderer/renderer.h"
#include "utils/allocators.h"

float abs(float a) {
    return a>0?a:-a;
}

void draw_line(Vector* framebuffer, uint32 w, Vertex* a, Vertex* b) {
    float dx = b->x-a->x;
    float dy = b->y-a->y;

    uint32 steps = abs(dx)>abs(dy) ? abs(dx) : abs(dy);

    float step_size_x = dx/steps;
    float step_size_y = dy/steps;

    // printf("steps: %d  dx: %.5f  dy: %.5f\n", steps, dx, dy);

    Vertex tmp = *a;
    for (uint32 s=0; s<steps; s++) {
        tmp.x += step_size_x;
        tmp.y += step_size_y;

        if (tmp.x<0 || tmp.y<0) {
            continue;
        }

        // uint8* pixel = vector_alloc_get(framebuffer, (uint32)(tmp.y*w) + (uint32)tmp.x);
        uint32* pixel = (uint32*)vector_alloc_get(framebuffer, 0);
        printf("pixel value before = %d\n", pixel);
        printf("pixel address: %p", pixel);
        *pixel = 0xFFFFFF;
        // printf("x0: %.3f \t x: %.3f \t x1: %.3f\n", a->x, tmp.x, b->x);
        // printf("y0: %.3f \t y: %.3f \t y1: %.3f\n", a->y, tmp.y, b->y);
    }
}

int main() {
    // syscalls: https://gpages.juszkiewicz.com.pl/syscalls-table/syscalls.html
    // 1 is write on x86_64
    const char msg[] = "Handmade something starts!\n";
    syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);

    char* file_path = "assets/backpack/backpack.obj";
    Model* model = read_model_file(file_path);

    uint16 w = 1024;
    uint16 h = 768;

    Vector* framebuffer = vector_alloc_create(w*h, sizeof(uint32));

    uint8 running = 1;
    while (running) {

        for (int i=0; i<vector_alloc_count(model->faces); ++i) {
            Face* f = (Face*)vector_alloc_get(model->faces, i);

            // Only use the x y components atm
            // Can work with perspective and camera later
            Vertex a = *(Vertex*)vector_alloc_get(model->vertices, f->v[0]-1);
            Vertex b = *(Vertex*)vector_alloc_get(model->vertices, f->v[1]-1);
            Vertex c = *(Vertex*)vector_alloc_get(model->vertices, f->v[2]-1);

            // Scale to center of the screen
            a.x = a.x * 0.5f * w;
            a.y = a.y * 0.5f * h;
            b.x = b.x * 0.5f * w;
            b.y = b.y * 0.5f * h;
            c.x = c.x * 0.5f * w;
            c.y = c.y * 0.5f * h;

            draw_line(framebuffer, w, &a, &b);
            draw_line(framebuffer, w, &b, &c);
            draw_line(framebuffer, w, &c, &a);
        }
        break;
    }


    free_model(model);
    vector_alloc_free(framebuffer);

    // platform_main();
}
