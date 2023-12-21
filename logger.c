#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "logger.h"

#define LOGGER_DEFAULT_TIME_FORMAT "[%d/%m/%Y - %H:%M:%S]"
#define LOGGER_MAX_ALLOC_LINE 1024

#define LOGGER_TRACE "TRACE"
#define LOGGER_DEBUG "DEBUG"
#define LOGGER_INFO "INFO"
#define LOGGER_WARN "WARN"
#define LOGGER_ERROR "ERROR"
#define LOGGER_FATAL "FATAL"
#define LOGGER_UNKNOWN "UNKNOWN"

typedef struct {
    char *filename;
    LoggerLevel level;
    int initialized;
} Logger;

static Logger logger = {0};

void LoggerInit(char *logFile, LoggerLevel level) {
    if (logger.initialized == 1) return;
    logger.filename = logFile;
    logger.level = level;
    logger.initialized = 1;
}

void LoggerSetLevel(LoggerLevel level) {
    logger.level = level;
}

char *_LoggerLevelToString(LoggerLevel level) {
    switch (level) {
        case L_TRACE: return LOGGER_TRACE;
        case L_DEBUG: return LOGGER_DEBUG;
        case L_INFO: return LOGGER_INFO;
        case L_WARN: return LOGGER_WARN;
        case L_ERROR: return LOGGER_ERROR;
        case L_FATAL: return LOGGER_FATAL;
    }
    return LOGGER_UNKNOWN;
}

void _LoggerWrite(LoggerLevel level, char *fmt, va_list ap) {
    if (logger.initialized == 0 || logger.level > level) return;
    FILE *fd = fopen(logger.filename, "a");
    
    // Get time and format
    time_t time_raw_format;
    struct tm *ptr_time;
    time (&time_raw_format);
    ptr_time = localtime(&time_raw_format);
    char *timebuf = calloc(50, sizeof(char));
    strftime(timebuf,50,LOGGER_DEFAULT_TIME_FORMAT,ptr_time);
    
    // Get level and format
    char *levelbuf = calloc(10, sizeof(char));
    snprintf(levelbuf, 10, "[%s] ", _LoggerLevelToString(level));

    // Build message and format
    char *buf = calloc(LOGGER_MAX_ALLOC_LINE, sizeof(char));
    vsnprintf(buf, LOGGER_MAX_ALLOC_LINE, fmt, ap);
    
    // Shift message to allocate space for "[TIME][LEVEL] Message..."
    memmove(buf + strlen(timebuf) + strlen(levelbuf), buf, strlen(buf));
    
    // Set level
    memmove(buf + strlen(timebuf), levelbuf, strlen(levelbuf));
    
    // Set format
    memmove(buf, timebuf, strlen(timebuf));

    // New line
    buf[strlen(buf) - 1] = '\n';

    // Writes to the file
    fwrite(buf, sizeof(char), strlen(buf), fd);
    va_end(ap);
    free(buf);
    fclose(fd);
}

void LoggerTrace(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    _LoggerWrite(L_TRACE, fmt, ap);
}

void LoggerDebug(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    _LoggerWrite(L_DEBUG, fmt, ap);
}

void LoggerInfo(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    _LoggerWrite(L_INFO, fmt, ap);
}

void LoggerWarn(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    _LoggerWrite(L_WARN, fmt, ap);
}

void LoggerError(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    _LoggerWrite(L_ERROR, fmt, ap);
}

void LoggerFatal(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    _LoggerWrite(L_FATAL, fmt, ap);
}
