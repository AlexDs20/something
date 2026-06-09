#if defined(ADS_LINUX)
    #include "platform/linux/memory.c"
#else
    #error "Only Linux support at the moment"
    void dummy_platform_memory(void);
#endif
