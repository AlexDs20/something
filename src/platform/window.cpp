#ifdef ADS_LINUX
#ifdef ADS_X11
// TODO(alex): Handle Xlib / XKB
#include "platform/linux/xlib.cpp"
#elif defined(ADS_WAYLAND)
// TODO(alex): Handle Wayland
#endif
// TODO(alex): Handle Windows + Mac
#endif
