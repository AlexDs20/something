#include <stdio.h>

#include "libs/ads_logger.h"
#include "base/base.h"

typedef struct {
    uint64_t length;
    uint8_t  log_level_bit;
    uint8_t  msg;
} Message;

typedef struct {
    const uint8_t* buffer;
    Message* first;
    int initialized;
} Logger;

static Logger _ads_logger = {0};

static void _initialize_logger(Logger* logger) {
}

void log_sv(LogLevel lvl, StringView sv) {
    Logger* logger = &_ads_logger;
    if (!logger->initialized) {
        _initialize_logger(logger);
        logger->initialized = 1;
    }

    if (1 /*enough_space*/) {
    }
    else {
    }

    return;
}

void print_log(LogLevel lvl) {
    // If not full
    // Fuck null-terminated strings
    // printf

    return;
}
