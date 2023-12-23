#ifndef LOGGER_H_
#define LOGGER_H_

#define LOG_TRACE(fmt, ...) LoggerLog(L_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) LoggerLog(L_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  LoggerLog(L_INFO , __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  LoggerLog(L_WARN , __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LoggerLog(L_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) LoggerLog(L_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum {
    L_TRACE = 0,
    L_DEBUG,
    L_INFO,
    L_WARN,
    L_ERROR,
    L_FATAL
} LogLevel;

void LoggerInit(char *logFile, LogLevel level);
void LoggerSetLevel(LogLevel level);
void LoggerLog(LogLevel level, const char *cfile, int line, char *fmt, ...);

#endif // LOGGER_H_