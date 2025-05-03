#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

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

    Visual* visual = DefaultVisual(display, screen);
    int depth = DefaultDepth(display, screen);

    // https://tronche.com/gui/x/xlib/window/attributes/
    unsigned long valuemask = CWBackPixel | CWEventMask | CWBitGravity;
    XSetWindowAttributes swa;
    swa.background_pixel = 0xFFFFFFFF;
    swa.bit_gravity = StaticGravity;        // Do not discard window data on resize

    // https://tronche.com/gui/x/xlib/events/mask.html
    // https://tronche.com/gui/x/xlib/events/processing-overview.html#ExposureMask
    swa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask;

    int border_width=0;
    Window window = XCreateWindow(display, root,
            x, y,
            w, h,
            border_width,
            depth, InputOutput,
            visual, valuemask, &swa
            );
    if (!window) {
        printf("Failed creating a window!\n");
        return(1);
    }

    XStoreName(display, window, title);
    XMapWindow(display, window);

    XWindowAttributes wa;
    XGetWindowAttributes(display, root, &wa);
    int max_w = wa.width;
    int max_h = wa.height;

    // Buffer image
    char pixel_bytes = 4;
    int* buffer = (int*)malloc(max_w*max_h*pixel_bytes);
    int offset = 0;
    int bitmap_pad = 32;
    int bytes_per_line = w*4;
    XImage* xim = XCreateImage(display, visual, depth, ZPixmap, offset, (char*)buffer, w, h, bitmap_pad, bytes_per_line);
    GC gc = DefaultGC(display, screen);
    XPutImage(display, window, gc, xim, 0, 0, 0, 0, w, h);

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
                    XPutImage(display, window, gc, xim, 0, 0, 0, 0, w, h);
                } break;
                case DestroyNotify: {
                    if (event.xdestroywindow.window == window) {
                        running = 0;
                    }
                } break;
                case KeyPress: {
                    printf("Key pressed!\n");
                    if (event.xkey.keycode == XKeysymToKeycode(display, XK_f)) {
                        toggle_fullscreen(display, window);
                    }
                } break;
                case KeyRelease: {
                    printf("Key released!\n");
                } break;
                case ConfigureNotify: {
                    // Go through all the ConfigureNotify events in the queue (they get removed from the queue)
                    // end we process the last event that was in the queue
                    XEvent next_event;
                    while (XCheckTypedEvent(display, ConfigureNotify, &next_event)) {
                        event = next_event;
                    }
                    int new_w = event.xconfigure.width;
                    int new_h = event.xconfigure.height;
                    if (new_w != w || new_h != h) {
                        w = new_w;
                        h = new_h;
                        printf("Window size: (%d,%d) = %.3f MB\n", w, h, (float)(w*h*pixel_bytes/1024.0f/1024.0f));

                        // set the data to null so that the DestroyImage doesn't free the buffer
                        xim->data = 0;
                        XDestroyImage(xim);
                        if (w*h>max_w*max_h) {
                            buffer = (int*)malloc(w*h*pixel_bytes);
                        }
                        xim = XCreateImage(display, visual, depth, ZPixmap, 0, (char*)buffer, w, h, bitmap_pad, w*4);
                    }
                } break;
                case ClientMessage: {
                    if (event.xclient.data.l[0] == wm_delete_window) {
                        running = 0;
                    }
                } break;
            }
        }

        // Write over the buffer
        for (int i=0; i<w*h; ++i) {
            int* p = buffer + i;
            if (i % w < w / 3) {
                *p = 0;
            } else if (i%w>= w/3 && i%w < 2*w/3) {
                *p = 0x00FFFF00;
            } else {
                *p = 0x00FF0000;
            }
        }

        // Draw the buffer
        XPutImage(display, window, gc, xim, 0, 0, 0, 0, w, h);
    }

    XDestroyWindow(display, window);
    xim->data = 0;
    XFree(xim);
    free(buffer);
    return(0);
}
