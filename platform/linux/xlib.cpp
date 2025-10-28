#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

#include "platform/window.h"
#include "utils/defines.h"

// https://handmade.network/forums/articles/t/2834-tutorial_a_tour_through_xlib_and_related_technologies

int set_fullscreen(Display* display, Window window, bool fullscreen) {
    // https://specifications.freedesktop.org/wm-spec/1.3/ar01s05.html#id-1.6.8
    //  Info about the format towards the end of the section
    XClientMessageEvent ev;
    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    Atom fullscreen_atom = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

    if (wm_state == None) {
        return(1);
    }

    ev.type = ClientMessage;
    ev.format = 32;
    ev.window = window;
    ev.message_type = wm_state;
    ev.data.l[0] = fullscreen ? 1 : 0;      // _NET_WM_STATE_REMOVE 0   _NET_WM_STATE_ADD 1   _NET_WM_STATE_TOGGLE 2
    ev.data.l[1] = fullscreen_atom;
    ev.data.l[2] = 0;
    ev.data.l[3] = 1;                       // normal applications 1   pagers and other clients 2

    // Send the event
    int success = XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask | SubstructureRedirectMask, (XEvent*)&ev);
    if (!success) {
        printf("Failed sending the toggle fullscreen event!\n");
        return(1);
    }

    return(0);
}

int toggle_fullscreen(Display* display, Window window) {
    // https://specifications.freedesktop.org/wm-spec/1.3/ar01s05.html#id-1.6.8
    //  Info about the format towards the end of the section
    XClientMessageEvent ev;
    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    Atom fullscreen_atom = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

    if (wm_state == None) {
        return(1);
    }

    ev.type = ClientMessage;
    ev.format = 32;
    ev.window = window;
    ev.message_type = wm_state;
    ev.data.l[0] = 2;                       // _NET_WM_STATE_REMOVE 0   _NET_WM_STATE_ADD 1   _NET_WM_STATE_TOGGLE 2
    ev.data.l[1] = fullscreen_atom;
    ev.data.l[2] = 0;
    ev.data.l[3] = 1;                       // normal applications 1   pagers and other clients 2

    // Send the event
    int success = XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask | SubstructureRedirectMask, (XEvent*)&ev);
    if (!success) {
        printf("Failed sending the toggle fullscreen event!\n");
        return(1);
    }

    return(0);
}

// TODO: Add support for RGB and GREY currently only RGBA
Win platform_init_win(unsigned int w, unsigned int h, char* title) {
    Win win = {};
    win.w = w;
    win.h = h;

    win.display = XOpenDisplay(0);
    if (!win.display) {
        printf("Failed opening display!\n");
        win = {};
        return(win);
    }

    int screen = DefaultScreen(win.display);
    int root = RootWindow(win.display, screen);

    int screen_bit_depth = 24;
    XVisualInfo vis_info;
    Status status = XMatchVisualInfo(win.display, screen, screen_bit_depth, TrueColor, &vis_info);
    if(status == 0) {
      printf("No matching visual info\n");
      win = {};
      return(win);
    }
    win.visual = vis_info.visual;
    win.depth = vis_info.depth;

    /*
    win.visual = DefaultVisual(win.display, screen);
    win.depth = DefaultDepth(win.display, screen);
    */

    // https://tronche.com/gui/x/xlib/window/attributes/
    unsigned long valuemask = CWBackPixel | CWColormap | CWEventMask | CWBitGravity;
    XSetWindowAttributes swa;
    swa.background_pixel = 0;   /* background fill pixel color */
    swa.colormap = XCreateColormap(win.display, root, vis_info.visual, AllocNone);
    swa.bit_gravity = StaticGravity;
    swa.event_mask  = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask;
    /*
    unsigned long valuemask = CWBackPixel | CWEventMask | CWBitGravity;
    swa.background_pixel = 0xFFFFFFFF;
    swa.bit_gravity = StaticGravity;        // Do not discard window data on resize

    // https://tronche.com/gui/x/xlib/events/mask.html
    // https://tronche.com/gui/x/xlib/events/processing-overview.html#ExposureMask
    swa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask;
    */

    int border_width=0;
    int x=0, y=0;
    win.window = XCreateWindow(
            win.display, root,
            x, y,
            win.w, win.h,
            border_width,
            win.depth, InputOutput,
            win.visual, valuemask, &swa
            );

    if (!win.window) {
        printf("Failed creating a window!\n");
        XCloseDisplay(win.display);
        win = {};
        return(win);
    }

    XStoreName(win.display, win.window, title);
    XMapWindow(win.display, win.window);
    XFlush(win.display);

    XWindowAttributes wa;
    XGetWindowAttributes(win.display, root, &wa);

    win.arena = arena_alloc_create(1*GiB);

    // Buffer image
    win.pixel_bytes = 4;
    win.buffer = (unsigned int*)arena_alloc_push(win.arena, wa.width*wa.height*win.pixel_bytes);

    int offset = 0;
    win.bitmap_pad = 32;
    int bytes_per_line = w*4;
    win.xim = XCreateImage(
            win.display,
            win.visual,
            win.depth,
            ZPixmap,
            offset,
            (char*)win.buffer,
            win.w,
            win.h,
            win.bitmap_pad,
            0);

    win.xim->data = (char*)win.buffer;

    win.gc = DefaultGC(win.display, screen);
    XPutImage(win.display, win.window, win.gc, win.xim, 0, 0, 0, 0, win.w, win.h);

    win.wm_delete_window = XInternAtom(win.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(win.display, win.window, &win.wm_delete_window, 1);

    return(win);
}

static
int trailing_zero_bits(u32 x) {
    /*
       if gcc/clang
        return __builtin_ctz(x);
       if msvc
       int n;
       int status = _BitScanForward(n, x);
       if (status==0) fail (return 32?);
       else return n;
    */
    if (x == 0) return 32;
    int n = 0;
    while ((x & 1) == 0) {
        x >>= 1;
        n++;
    }
    return n;
}

void platform_render_to_window(u8* buffer, u32 width, u32 height, u32 bits_per_pixels, Win* window) {
    // Buffer image format is RGBA
    u8* src = buffer;
    u8* dst = (u8*)window->buffer;

    // Reorder RGBA to X11 format
    u32* data = (u32*) buffer;
    // window->buffer is u32*

    int rshift = trailing_zero_bits(window->visual->red_mask);
    int gshift = trailing_zero_bits(window->visual->green_mask);
    int bshift = trailing_zero_bits(window->visual->blue_mask);

    // Note: do the shift AFTER so that if interpolating => do not need to do it on alpha
    for (int i=0; i<width*height; i++) {
        u8* p = (u8*)(data+i);
        u8 r = p[0];
        u8 g = p[1];
        u8 b = p[2];
        u8 a = p[3];
        data[i] = r<<rshift | g<<gshift | b<<bshift;
    }

    // Copy data from the buffer to the window
    u8 SCALED = 1;
    u8 NEAREST = 1;
    u8 LINEAR = 0;

    // If not scalled => simply copy the data as they are into the buffer used for the ximage
    if (SCALED == 0) {
        for (u32 j=0; j<window->h; j++) {
            memcpy(window->buffer+j*window->w, buffer+j*width*4, window->w*4);
        }
    } else {
        f32 w_ratio = (f32)width  / (f32)window->w;
        f32 h_ratio = (f32)height / (f32)window->h;
        if (NEAREST == 1) {
            for (u32 j=0; j<window->h; j++) {
                for (u32 i=0; i<window->w; i++) {
                    u64 idx = j*window->w + i;
                    u32 img_i = (i * w_ratio);
                    u32 img_j = (j * h_ratio);
                    img_i = img_i>width-1 ? width-1 : img_i;
                    img_j = img_j>height-1 ? height-1 : img_j;
                    u64 img_idx = img_j*width*4 + img_i*4;
                    window->buffer[idx] = *(u32*)(&buffer[img_idx]);
                }
            }
        } else if (LINEAR == 1) {
            for (u32 j=0; j<window->h; j++) {
                for (u32 i=0; i<window->w; i++) {
                }
            }
        } else {
        }
    }

    // And put the image
    XPutImage(window->display, window->window, window->gc, window->xim, 0, 0, 0, 0, window->w, window->h);
}

void print_event_type(int event_type) {
        switch (event_type) {
            case KeyPress: {
                printf("\t KeyPress\n");
            } break;
            case KeyRelease: {
                printf("\t KeyRelease\n");
            } break;
            case ButtonPress: {
                printf("\t ButtonPress\n");
            } break;
            case ButtonRelease: {
                printf("\t ButtonRelease\n");
            } break;
            case MotionNotify: {
                printf("\t MotionNotify\n");
            } break;
            case EnterNotify: {
                printf("\t EnterNotify\n");
            } break;
            case LeaveNotify: {
                printf("\t LeaveNotify\n");
            } break;
            case FocusIn: {
                printf("\t FocusIn\n");
            } break;
            case FocusOut: {
                printf("\t FocusOut\n");
            } break;
            case KeymapNotify: {
               printf("\t KeymapNotify\n");
            } break;
            case Expose: {
                printf("\t Expose\n");
            } break;
            case GraphicsExpose: {
                printf("\t GraphicsExpose\n");
            } break;
            case NoExpose: {
                printf("\t NoExpose\n");
            } break;
            case VisibilityNotify: {
                printf("\t VisibilityNotify\n");
            } break;
            case CreateNotify: {
                printf("\t CreateNotify\n");
            } break;
            case DestroyNotify: {
                printf("\t DestroyNotify\n");
            } break;
            case UnmapNotify: {
                printf("\t UnmapNotify\n");
            } break;
            case MapNotify: {
                printf("\t MapNotify\n");
            } break;
            case MapRequest: {
                printf("\t MapRequest\n");
            } break;
            case ReparentNotify: {
                printf("\t ReparentNotify\n");
            } break;
            case ConfigureNotify: {
                printf("\t ConfigureNotify\n");
            } break;
            case ConfigureRequest: {
                printf("\t ConfigureRequest\n");
            } break;
            case GravityNotify: {
                printf("\t GravityNotify\n");
            } break;
            case ResizeRequest: {
                printf("\t ResizeRequest\n");
            } break;
            case CirculateNotify: {
                printf("\t CirculateNotify\n");
            } break;
            case CirculateRequest: {
                printf("\t CirculateRequest\n");
            } break;
            case PropertyNotify: {
                printf("\t PropertyNotify\n");
            } break;
            case SelectionClear: {
                printf("\t SelectionClear\n");
            } break;
            case SelectionRequest: {
                printf("\t SelectionRequest\n");
            } break;
            case SelectionNotify: {
                printf("\t SelectionNotify\n");
            } break;
            case ColormapNotify: {
                printf("\t ColormapNotify\n");
            } break;
            case ClientMessage: {
                printf("\t ClientMessage\n");
            } break;
            case MappingNotify: {
                printf("\t MappingNotify\n");
            } break;
            case GenericEvent: {
               printf("\t GenericEvent\n");
           } break;
        }
}

bool platform_handle_events(Win* win) {
    int running = 1;
    XEvent event;
    while(XPending(win->display)>0) {
        XNextEvent(win->display, &event);
        // print_event_type(event.type);
        switch (event.type) {
            case Expose: {
                XPutImage(win->display, win->window, win->gc, win->xim, 0, 0, 0, 0, win->w, win->h);
            } break;
            case DestroyNotify: {
                if (event.xdestroywindow.window == win->window) {
                    running = 0;
                }
            } break;
            case KeyPress: {
                printf("Key pressed!\n");
                if (event.xkey.keycode == XKeysymToKeycode(win->display, XK_f)) {
                    toggle_fullscreen(win->display, win->window);
                }
            } break;
            case KeyRelease: {
                printf("Key released!\n");
            } break;
            case ConfigureNotify: {
                // Go through all the ConfigureNotify events in the queue (they get removed from the queue)
                // end we process the last event that was in the queue
                XEvent next_event;
                while (XCheckTypedEvent(win->display, ConfigureNotify, &next_event)) {
                    event = next_event;
                }
                int new_w = event.xconfigure.width;
                int new_h = event.xconfigure.height;
                if (new_w != win->w || new_h != win->h) {
                    win->w = new_w;
                    win->h = new_h;
                    printf("Window size: (%d,%d) = %.3f MB\n", win->w, win->h, (float)(win->w*win->h*win->pixel_bytes/1024.0f/1024.0f));

                    // set the data to null so that the DestroyImage doesn't free the buffer
                    win->xim->data = 0;
                    XDestroyImage(win->xim);

                    win->xim = XCreateImage(win->display, win->visual, win->depth, ZPixmap, 0, NULL, win->w, win->h, win->bitmap_pad, win->w*4);
                    arena_alloc_reset(win->arena);
                    win->buffer = (unsigned int*)arena_alloc_push(win->arena, win->w*win->h*win->pixel_bytes);
                    win->xim->data = (char*)win->buffer;
                }
            } break;
            case ClientMessage: {
                if (event.xclient.data.l[0] == win->wm_delete_window) {
                    running = 0;
                }
            } break;
        }
    }   // end of while XPending
    return(running);
}

void platform_cleanup_window(Win win) {
    if (win.xim) {
        win.xim->data = NULL; // Prevent XDestroyImage from trying to free arena memory
        XDestroyImage(win.xim);
    }
    arena_alloc_free(win.arena);
    XDestroyWindow(win.display, win.window);
    XCloseDisplay(win.display);
}
