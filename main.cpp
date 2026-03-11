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
#include "libs/ads_model_loader.h"

#include "gf_profiling.c"

int main(int argc, char** argv) {
    //
    // syscalls: https://gpages.juszkiewicz.com.pl/syscalls-table/syscalls.html
    // 1 is write on x86_64
    char msg[] = "Handmade something starts!\n";
    syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);

    Arena* global_arena = arena_alloc_create(1*GiB);
    Arena* scene_arena = arena_alloc_create(1*GiB);
    Arena* frame_arena = arena_alloc_create(1*GiB);

    // string8 file_path = string_from_cstr(scene_arena, "assets/backpack/backpack.obj");
    // string8 file_path = string_from_cstr(scene_arena, "../../../Downloads/006 - Charizard/BR_Charizard.obj");
    // string8 file_path = string_from_cstr(scene_arena, "../../../Downloads/dragon/dragon.obj");
    // string8 file_path = string_from_cstr(scene_arena, "../../../Downloads/sponza/sponza.obj");

    StringView selection = sv_from_cstr(argv[1]);

    StringView fp;
    if ( sv_equal(selection, sv_from_cstr("1")) ) {
        fp = sv_from_cstr("../../../Downloads/006 - Charizard/BR_Charizard.obj");
    } else if ( sv_equal(selection, sv_from_cstr("2")) ) {
        fp = sv_from_cstr("../../../Downloads/dragon/dragon.obj");
    } else if ( sv_equal(selection, sv_from_cstr("3")) ) {
        fp = sv_from_cstr("../../../Downloads/sponza/sponza.obj");
    } else if  ( sv_equal(selection, sv_from_cstr("4")) ) {
        fp = sv_from_cstr("assets/backpack/backpack.obj");
    } else {
        fp = sv_from_cstr("assets/cube.obj");
    }
    ObjModel* model = model_read(scene_arena, fp);
    // Model* model = {0};
    // if (string_get_file_extension(file_path) == ".obj") {
    //     model = read_obj_model_file(scene_arena, file_path);
    // }

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

    ColorContext frag_context = {
        0xFFFFA500,
    };

    // TextureContext frag_context = {
    //     .texture = &model->material->map_Kd,
    // };

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
        // draw_model(model, canvas_w, canvas_h, win_buffer, zbuffer, (void*) (&frag_context), shader_frag_texture);
        // draw_model(model, canvas_w, canvas_h, win_buffer, zbuffer, (void*) (&frag_context), shader_frag_depth);
        draw_model(model, canvas_w, canvas_h, win_buffer, zbuffer, (void*) (&frag_context), shader_frag_color);

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
