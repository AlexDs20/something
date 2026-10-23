#include <unistd.h>         // STDOUT_FILENO
#include <sys/syscall.h>    // syscall()

// #ifndef _POSIX_C_SOURCE
// #define _POSIX_C_SOURCE 200809L     // Define to expose POSIX functions: clock_gettime
// #endif

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "libs/ads_math.h"
#include "memory/allocators.h"
#include "platform/window.h"
#include "renderer/renderer.h"
#include "platform/input.h"

#include "libs/ads_string.h"
#include "libs/ads_model_loader.h"
#include "libs/ads_3d_camera.h"

#include "gf_profiling.c"

int main(int argc, char** argv) {
    const char* usage = "   \
        Usage: \
        ./main [path-to-obj] \
    \n";

    // syscalls: https://gpages.juszkiewicz.com.pl/syscalls-table/syscalls.html
    // 1 is write on x86_64
    char msg[] = "Handmade something starts!\n";
    syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);

    Arena* global_arena = arena_alloc_create(1*GiB);
    Arena* scene_arena = arena_alloc_create(1*GiB);
    Arena* frame_arena = arena_alloc_create(1*GiB);

    StringView fp;
    int args = argc - 1;
    if (args == 0) {
        fp = sv_from_cstr("assets/backpack/backpack.obj");
    }
    else if (args == 1) {
        fp = sv_from_cstr(argv[1]);
    }
    else {
        fprintf(stderr, "\nToo many input arguments.");
        fprintf(stderr, "\n%s", usage);
        return -1;
    }

    // Model* model = model_read(scene_arena, fp);
    Scene* scene = model_read(scene_arena, fp);

    u32 canvas_w = 1920;
    u32 canvas_h = 1080;
    const u32 bg_color = 0x0000182B;

    // TODO: Add support for RGB and GREY currently only RGBA
    Win win = platform_init_win(1920, 1080, "Handmade something!", ADSV_NEAREST);

    const f32 NINF = f32_ninf();

    u32 running = 1;

    Camera camera = camera_init(ADS_CAMERA_FREE_FLIGHT);
    f32x4x4 clip_from_view = camera_get_projection_matrix(&camera);   // As long as we do not change the aspect ratio etc, it's constant

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    s32x2 mouse_pos = {0, 0};
    bool mouse_pos_initialized = false;

    while (running) {
        // Handle time:
        struct timespec new_time;
        clock_gettime(CLOCK_MONOTONIC, &new_time);

        double dt = (new_time.tv_sec - current_time.tv_sec)
                  + (new_time.tv_nsec - current_time.tv_nsec) * 1e-9;

        current_time = new_time;

        arena_alloc_reset_zero(frame_arena);
        running = platform_handle_events(&win);

        // f32 dzoom = 0.0f;
        // const f32 ZOOM = 1.0f;

        // Keyboard motion
        if (ads_input_key_pressed(ADS_KEY_W) || ads_input_key_down(ADS_KEY_W)) {
            camera_move(&camera, ADS_MOTION_DIRECTION_FORWARD, (f32)dt);
        }
        if (ads_input_key_pressed(ADS_KEY_S) || ads_input_key_down(ADS_KEY_S)) {
            camera_move(&camera, ADS_MOTION_DIRECTION_BACKWARD, (f32)dt);
        }
        if (ads_input_key_pressed(ADS_KEY_A) || ads_input_key_down(ADS_KEY_A)) {
            camera_move(&camera, ADS_MOTION_DIRECTION_LEFT, (f32)dt);
        }
        if (ads_input_key_pressed(ADS_KEY_D) || ads_input_key_down(ADS_KEY_D)) {
            camera_move(&camera, ADS_MOTION_DIRECTION_RIGHT, (f32)dt);
        }

        // // Mouse wheel
        // if (ads_input_mouse_released(ADS_MOUSE4)) {
        //     dzoom = ZOOM;
        // }
        // else if (ads_input_mouse_released(ADS_MOUSE5)) {
        //     dzoom = -ZOOM;
        // }

        s32x2 new_mouse_pos = {
            ads_input_mouse_x(),
            ads_input_mouse_y()
        };

        if (mouse_pos_initialized) {
            s32x2 dx = s32x2_sub(new_mouse_pos, mouse_pos);
            camera_rotate(&camera, dx);
        }
        else {
            mouse_pos_initialized = true;
        }
        mouse_pos = new_mouse_pos;

        f32x4x4 view_from_world = camera_get_view_matrix(&camera);
        f32x4x4 clip_from_world = f32x4x4_mul(&clip_from_view, &view_from_world);

        // Allocate the canvas size we want to draw onto
        // It does not have to be the same size as the window
        canvas_w = win.w;
        canvas_h = win.h;

        f32* zbuffer = (f32*)arena_alloc_push(frame_arena, canvas_w*canvas_h*sizeof(f32));
        u32* win_buffer = (u32*)arena_alloc_push(frame_arena, canvas_w*canvas_h*sizeof(f32));

        // DATA format: [RR] [GG] [BB] [AA]
        // on little-endian: 0xAABBGGRR
        // on big-endian: 0xRRGGBBAA

        for (u64 i=0; i<canvas_h*canvas_w; i++) {
            win_buffer[i] = bg_color;
            zbuffer[i] = NINF;
        }

        // draw_model_wireframe(model, canvas_w, canvas_h, win_buffer);
        // draw_scene(scene, view, win_buffer, zbuffer, canvas_w, canvas_h);
        draw_scene(scene, &clip_from_world, win_buffer, zbuffer, canvas_w, canvas_h);
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

    return 0;
}
