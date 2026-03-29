#include <unistd.h>         // STDOUT_FILENO
#include <sys/syscall.h>    // syscall()
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "base/base.h"
#include "libs/ads_math.h"
#include "memory/allocators.h"
#include "platform/window.h"
#include "renderer/renderer.h"
#include "libs/ads_images.h"
#include "platform/memory.h"

#include "libs/ads_string.h"
#include "libs/ads_model_loader.h"

#include "gf_profiling.c"

struct Transform {
    f32x3 position;
    f32x3 scale;
    Quaternion rotation;
};

f32x4x4 f32x4x4_from_transform(const Transform& t) {
    f32x4x4 S = f32x4x4_scale_f32x3(t.scale);
    f32x4x4 R = f32x4x4_from_quat(t.rotation);
    f32x4x4 T = f32x4x4_translate(t.position);
    return T * R * S;
}

int main(int /*argc*/, char** argv) {
    //
    // syscalls: https://gpages.juszkiewicz.com.pl/syscalls-table/syscalls.html
    // 1 is write on x86_64
    char msg[] = "Handmade something starts!\n";
    syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);

    Arena* global_arena = arena_alloc_create(1*GiB);
    Arena* scene_arena = arena_alloc_create(1*GiB);
    Arena* frame_arena = arena_alloc_create(1*GiB);

    StringView selection = sv_from_cstr(argv[1]);

    StringView fp;
    // TODO: Have platform filesystem support
    if ( sv_equal(selection, sv_from_cstr("1")) ) {
        fp = sv_from_cstr("assets/backpack/backpack.obj");
    } else if ( sv_equal(selection, sv_from_cstr("2")) ) {
        fp = sv_from_cstr("ignore/assets/sponza/sponza.obj");
    } else if ( sv_equal(selection, sv_from_cstr("3")) ) {
        fp = sv_from_cstr("ignore/assets/fireplace/fireplace_room.obj");
    } else if  ( sv_equal(selection, sv_from_cstr("4")) ) {
        fp = sv_from_cstr("ignore/assets/dragon/dragon.obj");
    } else if  ( sv_equal(selection, sv_from_cstr("5")) ) {
        fp = sv_from_cstr("ignore/assets/gallery/gallery.obj");
    } else {
        fp = sv_from_cstr("assets/backpack/backpack.obj");
    }

    ObjModel* model = model_read(scene_arena, fp);

    Transform t;
    t.position = f32x3_make(0.5f, 0.4f, 0.0f);
    t.scale    = f32x3_make(0.15f, 0.15f, 0.15f);
    t.rotation = quat_make_rotation(f32x3_make(0.0f, 1.0f, 0.0f), 0.0f);

    f32x4x4 world = f32x4x4_from_transform(t);
    debug_print_f32x4x4(world);

    // u32* win_buffer = model->material->map_Kd.buffer;
    // u32 canvas_w = model->material->map_Kd.width;
    // u32 canvas_h = model->material->map_Kd.height;

    u32 canvas_w = 1920;
    u32 canvas_h = 1080;
    const u32 bg_color = 0x18181B;
    // TODO: Add support for RGB and GREY currently only RGBA
    Win win = platform_init_win(1920, 1080, "Handmade something!", ADSV_NEAREST);

    const f32 ninf = f32_ninf();

    u32 running = 1;

    // ColorContext frag_context = {
    //     0xFFFFA500,
    // };

    TextureContext frag_context;
    frag_context.texture = &(model->materials[0].map_Kd);
    frag_context.world = &world;

    f32 theta = 0.0f;
    while (running) {
        t.rotation = quat_make_rotation(f32x3_make(0.0f, 1.0f, 0.0f), theta);
        theta += (F32_TAU / 120.0f);
        world = f32x4x4_from_transform(t);
        // frag_context.world = &wor
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

        for (u64 i=0; i<canvas_h*canvas_w; i++) {
            win_buffer[i] = bg_color;
            zbuffer[i] = ninf;
        }

        // draw_model_wireframe(model, canvas_w, canvas_h, win_buffer);
        draw_model(model, canvas_w, canvas_h, win_buffer, zbuffer, (void*) (&frag_context), shader_frag_texture);
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
}
