#if defined(_LINUX)
#if defined(_X11)
// TODO(alex): Handle Xlib / XKB
#include "platform/linux/xlib.cpp"
#elif defined(_WAYLAND)
// TODO(alex): Handle Wayland
#endif
// TODO(alex): Handle Windows + Mac
#endif
