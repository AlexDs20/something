
# Platform
## Window

### Linux
- [ ] Xlib
    - [x] Windowing
    - [ ] keyboard events
    - [ ] mouse events
    - [ ] double buffering [link](https://www.x.org/releases/X11R7.7/doc/libXext/dbelib.html)
    - [ ] Use shared memory?
    - [ ] copy/paste [link](https://handmade.network/forums/articles/t/8544-implementing_copy_paste_in_x11)

- [ ] Xlib + opengl? [eg](https://alexvia.com/post/002_initializing_opengl_on_x11/)
- [ ] Wayland
- [ ] X11 without xlib [here](https://hereket.com/posts/from-scratch-x11-windowing/#writing-text)

- [ ] Support for any buffer type: RGB, RGBA, GREY
- [ ] Support for multiple windows

X11 info:
- [pdf reference xlib](https://www.x.org/docs/X11/xlib.pdf)
- [Event masks](https://tronche.com/gui/x/xlib/events/mask.html)
- [Event masks and event types](https://tronche.com/gui/x/xlib/events/processing-overview.html#ExposureMask)
- [Handmade network X11](https://handmade.network/forums/articles/t/2834-tutorial_a_tour_through_xlib_and_related_technologies)

### Windows

## Math
- [ ] Make a more complete math library with all basic functions like ceil, cos, sin, acos, ...

## Image
- [ ] Finish a good, simple and fast implementation to decode jpeg:
    - [ ] Baseline DCT
    - [ ] chroma subsampling
    - [ ] Progressive DCT
- [ ] Add decoding of png

## Audio
- [ ] [linux](https://handmade.network/forums/articles/t/8544-implementing_copy_paste_in_x11)

## Threading
- [ ] [windows?](https://handmade.network/forums/articles/t/7058-tutorial_multithreading)

# UI
- casey [imgui](https://caseymuratori.com/blog_0001)
- Sean barret [game dev mag sept. 2005 page 34](https://ubm-twvideo01.s3.amazonaws.com/o1/vault/GD_Mag_Archives/GDM_September_2005.pdf)
- Sean barret game dev mag sample code [link](https://silverspaceship.com/inner/imgui/)
- [nakst ui tuto](https://nakst.gitlab.io/tutorial/ui-part-1.html)
- [youtube clay](https://www.youtube.com/watch?v=DYWTw19_8r4)
- [vui](https://github.com/heroseh/vui/tree/main)
- Look at [microui](https://github.com/rxi/microui)

# Allocators
- [ ] [general arena](https://muit.xyz/posts/memory-implementing-a-general-arena/)

## Code style ([Eskil notes](https://www.youtube.com/watch?v=443UNeGrFoM)):
- MY_DEFINE
- MyType
- my_function()
- my_variable
- a = b         (with spaces)
- a == b

Define words
- array
- type
- node
- entity
- handle (opaque struct)
- func (function pointer)
- internal (to a module)

- i, j, k, count, length, found, next, previous, array, list, f(loat temporary), vec

file structure:
- module
- module/m_file....c
- module/ basically no .h except for internal usage
- module/module.h (with all for external user usage)

- module_group_object_create
- module_group_object_destroy (not object_remove)

application using API should get a handle which is a void* and should not need to know what it is. The user gets the handle from the API and uses it for the function calls to the api

In the internal.h file => the actual struct with everything

MACRO:
- __FILE__
- __LINE__
e.g.
extern void* f_debug_mem_malloc(uint size, char* file, uint line);
ifdef F_MEMORY_DEBUG
#define malloc(n) f_debug_mem_malloc(n, __FILE__, __LINE__);

double* a;

a = malloc((sizeof *a) * 10);
instead of using the type of a:
a = malloc(sizeof(double)*10);
