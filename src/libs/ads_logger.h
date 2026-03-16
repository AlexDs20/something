#ifndef _ADS_LOGGER_H_
#define _ADS_LOGGER_H_

#include "libs/ads_string.h"

#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE (1024*1024)
#endif

typedef enum {
    LOG_TRACE = 1 << 0
    LOG_DEBUG = 1 << 1,
    LOG_INFO  = 1 << 2,
    LOG_WARN  = 1 << 3,
    LOG_ERROR = 1 << 4,
    LOG_FATAL = 1 << 5,
} LogLevel;

void log_sv(LogLevel lvl, StringView sv);
void log_string(LogLevel lvl, String str);
#define ADS_LOG(level, cstr)   log_sv(level, sv_from_cstr(cstr))

#endif //_ADS_LOGGER_H_
