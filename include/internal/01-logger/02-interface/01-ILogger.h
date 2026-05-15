#ifndef ILOGGER_H
#define ILOGGER_H

#include <StandardDefines.h>
#include "../01-type/01-LogLevel.h"
#include "../01-type/02-Tag.h"

DefineStandardPointers(ILogger)
class ILogger {
    Public Virtual ~ILogger() = default;
    
    // Log level methods with tags and function name
    Public Virtual Void Log(CLogLevel logLevel, CTag tag, CStdString& message, CStdString& functionName) = 0;
    Public Virtual Void Debug(CTag tag, CStdString& message, CStdString& functionName) = 0;
    Public Virtual Void Verbose(CTag tag, CStdString& message, CStdString& functionName) = 0;
    Public Virtual Void Info(CTag tag, CStdString& message, CStdString& functionName) = 0;
    Public Virtual Void Warning(CTag tag, CStdString& message, CStdString& functionName) = 0;
    Public Virtual Void Error(CTag tag, CStdString& message, CStdString& functionName) = 0;
    Public Virtual Void Critical(CTag tag, CStdString& message, CStdString& functionName) = 0;
    
    // Log level methods with tags
    Public Virtual Void Log(CLogLevel logLevel, CTag tag, CStdString& message) = 0;
    Public Virtual Void Debug(CTag tag, CStdString& message) = 0;
    Public Virtual Void Verbose(CTag tag, CStdString& message) = 0;
    Public Virtual Void Info(CTag tag, CStdString& message) = 0;
    Public Virtual Void Warning(CTag tag, CStdString& message) = 0;
    Public Virtual Void Error(CTag tag, CStdString& message) = 0;
    Public Virtual Void Critical(CTag tag, CStdString& message) = 0;

    // Utility methods
    Public Virtual Void SetLogLevel(LogLevel level) = 0;
    Public NoDiscard Virtual LogLevel GetLogLevel() const = 0;
};

#endif // ILOGGER_H

