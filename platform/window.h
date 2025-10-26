#ifndef _PLATFORM_WINDOW_H
#define _PLATFORM_WINDOW_H
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "memory/allocators.h"

typedef struct {
    int w;
    int h;
    Display* display;
    Visual* visual;
    int depth;
    Window window;
    Arena* arena;
    char pixel_bytes;
    unsigned int* buffer;
    int bitmap_pad;
    XImage* xim;
    GC gc;
    Atom wm_delete_window;
} Win;

Win platform_init_win(unsigned int w, unsigned int h, char* title);
bool platform_handle_events(Win* win);
void platform_render_to_window(u8* buffer, u32 width, u32 height, u32 bits_per_pixels, Win* window);
void platform_cleanup_window(Win win);

#endif // _PLATFORM_WINDOW_H
