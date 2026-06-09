#ifdef ADS_LINUX
    #ifdef ADS_X11
        #include "platform/linux/xlib.c"
    #else
        #error "Only X11 support!"
    #endif
#else
    #error "Only linux support"
#endif
