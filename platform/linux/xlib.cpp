#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <string.h>

#include "utils/defines.h"

global_variable int Running = 0;


int platform_create_window(void) {
    // https://www.x.org/releases/X11R7.7/doc/man/man3/
    // https://x.org/releases/current/doc/libX11/libX11/libX11.html
    // https://www.x.org/releases/X11R7.7/doc/xproto/x11protocol.html
    // https://magcius.github.io/xplain/article/x-basics.html
    // https://hereket.com/posts/from-scratch-x11-windowing/

    Display *display;
    Window window;
    int screen;
    char *window_title = "Handmade Window";

    // Open a connection to the X server
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Cannot open display\n");
        return(0);
    }

    screen = DefaultScreen(display);

    XPixmapFormatValues *xpfv;
    int count_return;
    xpfv = XListPixmapFormats(display, &count_return);
    printf("xpfv: depth bpp sl: %d %d %d", xpfv->depth, xpfv->bits_per_pixel, xpfv->scanline_pad);

    // Create a simple window
    const int win_width = 1024;
    const int win_height = 768;
    unsigned int border_width = 0;
    unsigned long border = 0xFF0000;
    unsigned long background = 0x000000;
    window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                 0, 0,
                                 win_width, win_height,
                                 border_width,
                                 border,
                                 background);

    // Select event types we want to receive (e.g., expose events for drawing)
    XSelectInput(display, window,
            ExposureMask | ResizeRedirectMask |
            KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask
            // PointerMotionMask
            );

    // Protocol to handle window manager closing window => client kills window (press X)
    Atom delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &delete_atom, 1);

    // Set the window title
    XStoreName(display, window, window_title);

    // Map the window to make it visible
    XMapWindow(display, window);

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
    xim = XCreateImage(display, wa.visual, wa.depth,
         ZPixmap, offset,
         (char*)framebuffer,
         win_width, win_height,
         32, win_width*4);

    // Get Graphics Context
    GC gc = XDefaultGC(display, screen);
    // Easiest solution is to put the ximage on the window directly
    // XPutImage(display, window, gc, xim, 0, 0, 0, 0, win_width, win_height);
    // Another possibility is:
    //  Put the data on an Pixmap (offscreen image)
    //  And then copy from the pixmap to the window using XCopyArea
    Pixmap pixmap = XCreatePixmap(display, window, win_width, win_height, wa.depth);
    XPutImage(display, pixmap, gc, xim, 0, 0, 0, 0, win_width, win_height);
    XCopyArea(display, pixmap, window, gc, 0, 0, win_width, win_height, 0,0);

    XSetCloseDownMode(display, DestroyAll);

    // Event loop to handle events
    Running = 1;
    while(Running) {
        // See also XPending to get the number of events in the queue
        XEvent event;
        while (XPending(display) > 0) {
            XNextEvent(display, &event);
            switch (event.type) {
                case ClientMessage: {
                    if (event.xclient.data.l[0] == delete_atom) {
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
                        printf("Pressed key: %lu\n", XLookupKeysym(&event.xkey, 0));
                    }
                } break;

                case KeyRelease: {
                    printf("Released key: %lu\n", XLookupKeysym(&event.xkey, 0));
                } break;

                case ButtonPress: {
                    printf("ButtonPress: (x,y) = (%d,%d), state=%d, button=%d\n", event.xbutton.x, event.xbutton.y, event.xbutton.state, event.xbutton.button);
                } break;

                case ButtonRelease: {
                    printf("ButtonRelease: (x,y) = (%d,%d), state=%d, button=%d\n", event.xbutton.x, event.xbutton.y, event.xbutton.state, event.xbutton.button);
                } break;

                case MotionNotify: {
                    printf("Motion notification\n");
                } break;

                case ResizeRequest: {
                    printf("Resize to: (%d,%d)\n", event.xresizerequest.width, event.xresizerequest.height);
                } break;

                default: {
                    printf("Received event type: %d\n", event.type);
                }
            }
        }

        XPutImage(display, pixmap, gc, xim, 0, 0, 0, 0, win_width, win_height);
        XCopyArea(display, pixmap, window, gc, 0, 0, win_width, win_height, 0,0);
    }

    // Close the connection to the X server
    XCloseDisplay(display);
    XDestroyImage(xim);
    // free(framebuffer);       // freed by XDestroyImage
    return(1);
}
