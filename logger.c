#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "logger.h"

#define LOGGER_DEFAULT_TIME_FORMAT "%d/%m/%Y - %H:%M:%S"
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
    LogLevel level;
    int initialized;
} Logger;

static Logger logger = {0};

void LoggerInit(char *logFile, LogLevel level) {
    if (logger.initialized == 1) return;
    logger.filename = logFile;
    logger.level = level;
    logger.initialized = 1;
}

void LoggerSetLevel(LogLevel level) {
    logger.level = level;
}

char *_LoggerLevelToString(LogLevel level) {
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

void LoggerLog(LogLevel level, const char *cfile, int line, char *fmt, ...) {
    
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
    snprintf(levelbuf, 10, " [%s]", _LoggerLevelToString(level));

    // Get c-file and line
    char *file_data = calloc(20, sizeof(char));
    snprintf(file_data, 20, " %s:%d ", cfile, line);

    // Build message and format
    va_list ap;
    va_start(ap, fmt);
    char *buf = calloc(LOGGER_MAX_ALLOC_LINE, sizeof(char));
    vsnprintf(buf, LOGGER_MAX_ALLOC_LINE, fmt, ap);
    va_end(ap);
    
    // Shift message to allocate space for "[TIME][LEVEL] Message..."
    memmove(buf + strlen(timebuf) + strlen(levelbuf) + strlen(file_data), buf, strlen(buf));
    
    // Shift file data
    memmove(buf + strlen(timebuf) + strlen(levelbuf), file_data, strlen(file_data));
    
    // Set level
    memmove(buf + strlen(timebuf), levelbuf, strlen(levelbuf));
    
    // Set format
    memmove(buf, timebuf, strlen(timebuf));

    // New line
    buf[strlen(buf) - 1] = '\n';

    // Writes to the file
    fwrite(buf, sizeof(char), strlen(buf), fd);
    free(buf);
    fclose(fd);
}
