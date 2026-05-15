#ifndef LOG_LEVEL_H
#define LOG_LEVEL_H

#include <StandardDefines.h>

// Log level enum - ordered from lowest to highest priority
DefineStandardTypes(LogLevel)
enum class LogLevel {
    Debug,      // 0 - Most verbose, detailed debugging info
    Verbose,    // 1 - Verbose information
    Info,       // 2 - General information
    Warning,    // 3 - Warning messages
    Error,      // 4 - Error conditions
    Critical    // 5 - Critical failures
};

#endif // LOG_LEVEL_H

