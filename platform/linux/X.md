# Notes

- *screen*: a physical monitor
- *display*: a set of screens for a single user with 1 keyboard and 1 mouse (or a pointer)
- *windows*: arranged in a hierarchy with top parent => root covers each of the display screens
- Coordinate system X -> horizontal, Y -> vertical with (0,0) top left
    - Correspond to pixel center
    - For window, origin is in the border
- Server sends an *Expose* event when parts of the window needs to be redrawn
- *Pixmap*: are offscreen storage of graphics
    - Can often be used interchangeably with *window*
- *Drawable*: window and pixmap
- Requests send to the XServer can be async. If the client wants: *XSync* blocks until all is resolved
- Lots of the resources are sharable between applications!
- Display can have 12-bit depth for the color (pseudo-color) or 24-bits (8 for each RGB).
    - Called *Visuals*


- Open connection to the X server:
    - *XOpenDisplay*
- To make a window visible:
    - Map it on the screen: *XMapWindow*
- To set attribute of a window:
    - *XSetWindowAttributes* and bit map in later calls to *XCreateWindow* and *XChangeWindowAttributes*

