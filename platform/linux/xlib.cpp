#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform/window.h"
#include "utils/defines.h"

#define PLATFORM_KEY_NUM 0xFFFF
#define PLATFORM_POINTER_KEY_NUM 0xFFFFF

typedef struct {
    unsigned char pressed;
    unsigned char released;
    unsigned char down;
} Key;

typedef struct {
    int x;
    int y;
    Key buttons[PLATFORM_POINTER_KEY_NUM];
} Pointer;

typedef struct Platform {
    Display* display;
    Window window;
    int screen;
    Atom delete_atom;
    Key keys[PLATFORM_KEY_NUM];
    Pointer pointer;
} Platform;

// https://tronche.com/gui/x/xlib/
// https://www.x.org/releases/X11R7.7/doc/man/man3/
// https://x.org/releases/current/doc/libX11/libX11/libX11.html
// https://www.x.org/releases/X11R7.7/doc/xproto/x11protocol.html
// https://magcius.github.io/xplain/article/x-basics.html
// https://hereket.com/posts/from-scratch-x11-windowing/

int platform_create_window(Platform* p, char* window_title, int w, int h, int x, int y, unsigned long background_color) {
    p->window = XCreateSimpleWindow(p->display, RootWindow(p->display, p->screen),
                                 x, y,
                                 w, h,
                                 1,
                                 0x000000,
                                 background_color);

    // Select event types we want to receive (e.g., expose events for drawing)
    XSelectInput(p->display, p->window,
            ExposureMask | ResizeRedirectMask |
            KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask
            );

    // Protocol to handle window manager closing window => client kills window (press X)
    // p->delete_atom = XInternAtom(p->display, "WM_DELETE_WINDOW", False);
    // XSetWMProtocols(p->display, p->window, &p->delete_atom, 1);

    // Set the window title
    // XTextProperty window_title_property;
    // int rc = XStringListToTextProperty(&window_title, 1, &window_title_property);
    // XSetWMName(p->display, p->window, &window_title_property);
    // XSetWMIconName(p->display, p->window, &iconized_window_title_property);
    XStoreName(p->display, p->window, window_title);

    // Map the window to make it visible
    XMapWindow(p->display, p->window);
    XFlush(p->display);

    return(0);
}

int platform_handle_key(Platform* p, XEvent* event, int down) {
    KeySym v = XLookupKeysym(&event->xkey, 0);
    p->keys[(unsigned long)v].released = p->keys[(unsigned long)v].pressed && !down;
    p->keys[(unsigned long)v].pressed = !p->keys[(unsigned long)v].pressed && down;
    p->keys[(unsigned long)v].down = down;

    return(0);
}

int platform_handle_pointer_button(Platform* p, XEvent* event, int down) {
    unsigned int button = event->xbutton.button;
    p->pointer.buttons[button].released = p->keys[button].pressed && !down;
    p->pointer.buttons[button].pressed = !p->keys[button].pressed && down;
    p->pointer.buttons[button].down = down;

    p->pointer.x = event->xbutton.x;
    p->pointer.y = event->xbutton.y;
    return(0);
}

int platform_events(Platform* p) {
    XEvent event;
    while (XPending(p->display) > 0) {
        XNextEvent(p->display, &event);
        switch (event.type) {
            case KeyPress: {
                platform_handle_key(p, &event, true);
            } break;

            case KeyRelease: {
                platform_handle_key(p, &event, false);
            } break;

            case ButtonPress: {
                platform_handle_pointer_button(p, &event, true);
            } break;

            case ButtonRelease: {
                platform_handle_pointer_button(p, &event, false);
            } break;

            case MotionNotify: {
                platform_handle_pointer_position(p, &event);
            } break;

            case Expose: {
                local_variable XSetWindowAttributes xwa = {.background_pixel=0};
                if (xwa.background_pixel == 0) {
                    xwa.background_pixel = 0xFFFFFF;
                } else {
                    xwa.background_pixel = 0x0;
                }
                XChangeWindowAttributes(p->display, p->window, 1L<<1, &xwa);
                // UpdatePixmap();
                // DrawPixmapSize();
            } break;

            case ResizeRequest: {
                platform_handle_resize(p, &event);
            } break;

            case ClientMessage: {
                if (event.xclient.data.l[0] == XInternAtom(p->display, "WM_DELETE_WINDOW", False)) {
                    printf("Supposed to close\n");
                    // Running = 0;
                }
            } break;

            default: {
                // printf("Received event type: %d\n", event.type);
            }
        }
    }
    return(0);
}

int platform_main() {
    int x=0, y=0;
    int w=1024, h=768;
    char* title = "Handmade something";

    Display* display = XOpenDisplay(0);
    if (!display) {
        printf("Failed opening display!\n");
        return(1);
    }

    int screen = DefaultScreen(display);
    int root = DefaultRootWindow(display);

    XWindowAttributes wa;
    XGetWindowAttributes(display, root, &wa);

    // https://tronche.com/gui/x/xlib/window/attributes/
    unsigned long valuemask = CWBackPixel | CWEventMask;
    XSetWindowAttributes swa;
    swa.background_pixel = 0xFFFFFF;

    // https://tronche.com/gui/x/xlib/events/mask.html
    // https://tronche.com/gui/x/xlib/events/processing-overview.html#ExposureMask
    swa.event_mask = ExposureMask | StructureNotifyMask;

    Window window = XCreateWindow(display, root,
            x, y,
            w, h,
            0,
            wa.depth,
            InputOutput,
            wa.visual,
            valuemask,
            &swa
            );

    XMapWindow(display, window);
    XStoreName(display, window, title);



    bool running = true;
    while(running){
        XEvent event;
        while(XPending(display)>0) {
            XNextEvent(display, &event);
            switch (event.type) {
                case (Expose): {
                    printf("Expose\n");
                } break;
                case (DestroyNotify): {
                    if (event.xdestroywindow.window == window) {
                        running = False;
                    }
                } break;
            }
        }
    }
    return(0);
}

int platform_main_old() {
    int x=0, y=0;
    int w=1024, h=768;

    Display* display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Cannot open display\n");
        return(1);
    }
    int screen = DefaultScreen(display);

    int max_w = DisplayWidth(display, screen);
    int max_h = DisplayHeight(display, screen);
    unsigned long background_color = 0xBBBBBB;

    Window window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                 x, y,
                                 w, h,
                                 1,
                                 0x000000,
                                 background_color);

    XSelectInput(display, window,
            ExposureMask | ResizeRedirectMask
            );

    XStoreName(display, window, "SOMETHING!");
    XMapWindow(display, window);
    XFlush(display);

    XWindowAttributes wa = {0};
    XGetWindowAttributes(display, window, &wa);

    GC gc = DefaultGC(display, screen);

    //==============================
    // User side
    char* data = (char*)malloc(w*h*4);

    for (int i = 0; i < (w*h); i++) {
        int row = (int)i/w;
        int res = i - row * w;
        if (res < (int)w/3) {
            data[i*4] = 0x00;
        } else if ((int)w/3 <= res  && res < (int)2*w/3) {
            data[i*4+0] = 0x00;
            data[i*4+1] = 0xFF;
            data[i*4+2] = 0xFF;
            data[i*4+3] = 0x00;
        } else {
            data[i*4+0] = 0x00;
            data[i*4+1] = 0x00;
            data[i*4+2] = 0xFF;
            data[i*4+3] = 0x00;
        }
    }
    //==============================
    printf("Pixmap depth: %d\n", wa.depth);
    // Make the pixmap max size of my screen so don't have to allocate even on window resize
    Pixmap pixmap = XCreatePixmap(display, window, max_w, max_h, wa.depth);
    // Pixmap pixmap = XCreatePixmap(display, window, w, h, wa.depth);
    int offset = 0;
    XImage* xim = XCreateImage(display, wa.visual, wa.depth, ZPixmap, offset, (char *)data, w, h, 32, w*4);
    XPutImage(display, pixmap, gc, xim, 0, 0, 0, 0, w, h);
    XCopyArea(display, pixmap, window, gc, 0, 0, w, h, 0, 0);

    Pixmap* pixptr = &pixmap;

    int Running = 1;
    while(Running) {
        XEvent event;
        while (XPending(display)>0) {
            XNextEvent(display, &event);
            switch (event.type) {
                case Expose: {
                    printf("EXPOSE EVENT\n");
                    // Add method on how to redraw => nearest, interp, stretch
                    {
                        XClearWindow(display, window);
                        XPutImage(display, pixmap, gc, xim, 0, 0, 0, 0, w, h);
                        XCopyArea(display, pixmap, window, gc, 0, 0, w, h, 0, 0);
                    }
                } break;
                case ResizeRequest: {
                    int new_w = event.xresizerequest.width;
                    int new_h = event.xresizerequest.height;
                    printf("Resize to: (%d,%d)\n", new_w, new_h);
                    {
                        XClearWindow(display, window);
                        XPutImage(display, pixmap, gc, xim, 0, 0, 0, 0, new_w, new_h);
                        XCopyArea(display, pixmap, window, gc, 0, 0, new_w, new_h, 0, 0);
                    }
                } break;
            }
        }
    }

    free(data);
    XUnmapWindow(display, window);
    XCloseDisplay(display);

    return(0);
}


int platform_create_window(void) {
#if 0
    int error = platform_initialize();
    if (error) {
        printf("Failed initializing the platform\n");
        return(error);
    }

    platform_skapa_window(&p, 0, 0, 1024, 768, "Handmade Window!");
    // Supported Pixmap formats
    XPixmapFormatValues *xpfv;
    int count_return;
    xpfv = XListPixmapFormats(display, &count_return);
    printf("Supported pixmap formats: \n");
    for (int i=0; i<count_return; i++) {
        printf("xpfv[%d]: (depth,bpp,sl)=(%d,%d,%d)\n", i, xpfv[i].depth, xpfv[i].bits_per_pixel, xpfv[i].scanline_pad);
    }
    // XFree((void*)xpfv)

    // int order = ImageByteOrder(display);
    // order = LSBFirst || MSBFirst             // Least/Most SignificantBit (endianness)
    // BitmapBitOrder
    // BitmapPad(display)

    // Handle image to put on the window
    const int offset = 0;
    int *framebuffer = (int *)malloc(win_width*win_height*4);

    for (int i = 0; i < (win_width*win_height); i++) {
        int row = (int)i/win_width;
        int res = i - row * win_width;
        if (res < (int)win_width/3) {
            framebuffer[i] = 0;
        } else if ((int)win_width/3 <= res  && res < (int)2*win_width/3) {
            framebuffer[i] = 0xFFFFFF00;
        } else {
            framebuffer[i] = 0xFFFF0000;
        }
    }

    // To put an image we need an XImage and a GC
    // To create an XImage -> need a visual
    // Visual get be gotten from attribute of a window
    XWindowAttributes wa = {0};
    XGetWindowAttributes(display, window, &wa);
    XImage *xim;
    // DefaultVisual(display, screen);
    // DefaultDepth(display, depth);
    xim = XCreateImage(display, wa.visual, wa.depth,
         ZPixmap, offset,
         (char*)framebuffer,
         win_width, win_height,
         32, win_width*4);

    // Get Graphics Context
    GC gc = DefaultGC(display, screen);
    // Easiest solution is to put the ximage on the window directly
    // XPutImage(display, window, gc, xim, 0, 0, 0, 0, win_width, win_height);
    // Another possibility is:
    //  Put the data on an Pixmap (offscreen image)
    //  And then copy from the pixmap to the window using XCopyArea
    Pixmap pixmap = XCreatePixmap(display, window, win_width, win_height, wa.depth);
    XPutImage(display, pixmap, gc, xim, 0, 0, 0, 0, win_width, win_height);
    XCopyArea(display, pixmap, window, gc, 0, 0, win_width, win_height, 0, 0);


    XSetCloseDownMode(p.display, DestroyAll);

    // Event loop to handle events
    Running = 1;
    while(Running) {
        // See also XPending to get the number of events in the queue
        XEvent event;
        while (XPending(p.display) > 0) {
            XNextEvent(p.display, &event);
            switch (event.type) {
                case ClientMessage: {
                    if (event.xclient.data.l[0] == p.delete_atom) {
                        Running = 0;
                    }
                } break;
                case Expose: {
                    // UpdatePixmap();
                    // DrawPixmapSize();
                } break;

                case KeyPress: {
                    if (XLookupKeysym(&event.xkey, 0) == XK_q || XLookupKeysym(&event.xkey, 0) == XK_Escape) {
                        Running = 0;
                    } else {
                        // printf("Pressed key: %lu\n", XLookupKeysym(&event.xkey, 0));
                    }
                } break;

                case KeyRelease: {
                    // printf("Released key: %lu\n", XLookupKeysym(&event.xkey, 0));
                } break;

                case ButtonPress: {
                    // printf("ButtonPress: (x,y) = (%d,%d), state=%d, button=%d\n", event.xbutton.x, event.xbutton.y, event.xbutton.state, event.xbutton.button);
                } break;

                case ButtonRelease: {
                    // printf("ButtonRelease: (x,y) = (%d,%d), state=%d, button=%d\n", event.xbutton.x, event.xbutton.y, event.xbutton.state, event.xbutton.button);
                } break;

                case MotionNotify: {
                    // printf("Motion notification\n");
                } break;

                case ResizeRequest: {
                    // printf("Resize to: (%d,%d)\n", event.xresizerequest.width, event.xresizerequest.height);
                } break;

                default: {
                    // printf("Received event type: %d\n", event.type);
                }
            }
        }

        // XPutImage(display, pixmap, gc, xim, 0, 0, 0, 0, win_width, win_height);
        // XCopyArea(display, pixmap, window, gc, 0, 0, win_width, win_height, 0,0);
    }

    // Close the pection to the X server
    XUnmapWindow(p.display, p.window);
    XCloseDisplay(p.display);
    // XDestroyImage(xim);
    // free(framebuffer);       // freed by XDestroyImage
#endif
    return(0);
}
