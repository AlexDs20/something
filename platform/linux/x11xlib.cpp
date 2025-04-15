#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <string.h>
#include <unistd.h> // For sleep


#define global_variable static
#define local_variable static
#define internal_function static

global_variable int Running = 0;

int XWindow(void) {
    // https://www.x.org/releases/X11R7.7/doc/man/man3/
    // https://x.org/releases/current/doc/libX11/libX11/libX11.html
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

    // Create a simple window
    const int win_width = 1024;
    const int win_height = 768;
    unsigned int border_width = 0;
    unsigned long border = 0xFF0000;
    unsigned long background = 0x000000;
    window = XCreateSimpleWindow(display,
                                 RootWindow(display, screen),
                                 0, 0,
                                 win_width, win_height,
                                 border_width,
                                 border,
                                 background);

    // Select event types we want to receive (e.g., expose events for drawing)
    XSelectInput(display, window,
            ExposureMask |
            KeyPressMask |
            KeyReleaseMask |
            ResizeRedirectMask |
            ButtonPress |
            ButtonPressMask|
            PointerMotionMask |
            ButtonMotionMask);

    // Set the window title
    XStoreName(display, window, window_title);

    // Map the window to make it visible
    XMapWindow(display, window);

    // Event loop to handle events
    XEvent event;

    // Protocol to handle window manager closing window => client kills window (press X)
    Atom delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &delete_atom, 1);

    const int offset = 0;
    int *framebuffer = (int *)malloc(win_width*win_height*4);

    for (int i = 0; i < (win_width*win_height); i++)
    {
        if (i < (int)(win_width*win_height)/3) {
            framebuffer[i] = 0x00FFFF;
        } else if (i < (int)(2*win_width*win_height)/3) {
            framebuffer[i] = 0xFFFF00;
        } else {
            framebuffer[i] = 0xFF0000;
        }
    }

    // To put an image we need an XImage and a GC
    // To create an XImage -> need a visual
    // Visual get be gotten from attribute of a window
    XWindowAttributes wa = {0};
    XGetWindowAttributes(display, window, &wa);
    XImage *xim;
    xim = XCreateImage(display,
         wa.visual,
         wa.depth,
         ZPixmap,
         offset,
         (char*)framebuffer,
         win_width,
         win_height,
         32,
         win_width*4);

    // Get Graphics Context
    GC gc = XDefaultGC(display, screen);
    XPutImage(display, window, gc, xim, 0, 0, 0, 0, win_width, win_height);

    Running = 1;
    while(Running) {
        // See also XPending to get the number of events in the queue
        XNextEvent(display, &event);
        switch (event.type) {
            case ClientMessage: {
                if (event.xclient.data.l[0] == delete_atom) {
                    Running = 0;
                }
            } break;
            case Expose: {
                printf("DRAWING\n");
                XPutImage(display, window, gc, xim, 0, 0, 0, 0, win_width, win_height);
                XSync(display, False);
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

    // Close the connection to the X server
    XCloseDisplay(display);
    XDestroyImage(xim);
    // free(framebuffer);       // freed by XDestroyImage
    return(1);
}


int main() {
    XWindow();
    return (0);
}
