#ifndef PLATFORM_WINDOW_H
#define PLATFORM_WINDOW_H
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "base/base.h"

#include "memory/allocators.h"

//============
// DRAW_METHOD
#define ADSV_NOTHING    0
#define ADSV_NEAREST    1
#define ADSV_BILINEAR   2


typedef struct {
    unsigned int w;
    unsigned int h;
    Display* display;
    Visual* visual;
    int depth;
    Window window;
    Arena* arena;
    int draw_method;
    char pixel_bytes;
    unsigned int* buffer;
    int bitmap_pad;
    XImage* xim;
    GC gc;
    Atom wm_delete_window;
} Win;

Win platform_init_win(unsigned int w, unsigned int h, char* title, int DRAW_METHOD);
bool platform_handle_events(Win* win);
void platform_render_to_window(u8* buffer, u32 width, u32 height, Win* window);
void platform_cleanup_window(Win win);

#endif // PLATFORM_WINDOW_H
