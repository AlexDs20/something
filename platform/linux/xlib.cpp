#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

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
    if (!window) {
        printf("Failed creating a window!\n");
        return(1);
    }

    XStoreName(display, window, title);
    XMapWindow(display, window);

    // set_fullscreen(display, window, true);
    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    bool running = 1;
    while(running){
        XEvent event;
        while(XPending(display)>0) {
            XNextEvent(display, &event);
            switch (event.type) {
                case Expose: {
                } break;
                case DestroyNotify: {
                    if (event.xdestroywindow.window == window) {
                        running = 0;
                    }
                } break;
                case ClientMessage: {
                    if (event.xclient.data.l[0] == wm_delete_window) {
                        XDestroyWindow(display, window);
                        running = 1;
                    }
                } break;
            }
        }
    }
    return(0);
}
