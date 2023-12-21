#ifndef LOGGER_H_
#define LOGGER_H_

typedef enum {
    L_TRACE = 0,
    L_DEBUG,
    L_INFO,
    L_WARN,
    L_ERROR,
    L_FATAL
} LoggerLevel;

void LoggerInit(char *logFile, LoggerLevel level);
void LoggerSetLevel(LoggerLevel level);
void LoggerTrace(char *fmt, ...);
void LoggerDebug(char *fmt, ...);
void LoggerInfo(char *fmt, ...);
void LoggerWarn(char *fmt, ...);
void LoggerError(char *fmt, ...);
void LoggerFatal(char *fmt, ...);

#endif // LOGGER_H_