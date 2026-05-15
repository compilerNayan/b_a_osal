#ifndef LOGGER_INTERNAL_H
#define LOGGER_INTERNAL_H

#include <StandardDefines.h>
#include "../../02-interface/01-ILogger.h"
#include "../../02-interface/02-ILogSink.h"

/* @Component */
class Logger final : public ILogger {
    Private LogLevel currentLogLevel;
    
    /* @Autowired */
    Private ILogSinkPtr logSink;
    
    Public Logger() : currentLogLevel(LogLevel::Info) {}
    
    Public Void Log(CLogLevel logLevel, CTag tag, CStdString& message, CStdString& functionName) override {
        if (!ShouldLog(logLevel)) return;
        WriteLog(logLevel, tag, Tag::Untagged, message, functionName);
    }
    
    Public Void Debug(CTag tag, CStdString& message, CStdString& functionName) override {
        if (!ShouldLog(LogLevel::Debug)) return;
        WriteLog(LogLevel::Debug, tag, Tag::Untagged, message, functionName);
    }
    
    Public Void Verbose(CTag tag, CStdString& message, CStdString& functionName) override {
        if (!ShouldLog(LogLevel::Verbose)) return;
        WriteLog(LogLevel::Verbose, tag, Tag::Untagged, message, functionName);
    }
    
    Public Void Info(CTag tag, CStdString& message, CStdString& functionName) override {
        if (!ShouldLog(LogLevel::Info)) return;
        WriteLog(LogLevel::Info, tag, Tag::Untagged, message, functionName);
    }
    
    Public Void Warning(CTag tag, CStdString& message, CStdString& functionName) override {
        if (!ShouldLog(LogLevel::Warning)) return;
        WriteLog(LogLevel::Warning, tag, Tag::Untagged, message, functionName);
    }
    
    Public Void Error(CTag tag, CStdString& message, CStdString& functionName) override {
        if (!ShouldLog(LogLevel::Error)) return;
        WriteLog(LogLevel::Error, tag, Tag::Untagged, message, functionName);
    }
    
    Public Void Critical(CTag tag, CStdString& message, CStdString& functionName) override {
        if (!ShouldLog(LogLevel::Critical)) return;
        WriteLog(LogLevel::Critical, tag, Tag::Untagged, message, functionName);
    }
    
    Public Void Log(CLogLevel logLevel, CTag tag, CStdString& message) override {
        StdString functionName = "-";
        Log(logLevel, tag, message, functionName);
    }

    Public Void Debug(CTag tag, CStdString& message) override {
        StdString functionName = "-";
        Debug(tag, message, functionName);
    }

    Public Void Verbose(CTag tag, CStdString& message) override {
        StdString functionName = "-";
        Verbose(tag, message, functionName);
    }

    Public Void Info(CTag tag, CStdString& message) override {
        StdString functionName = "-";
        Info(tag, message, functionName);
    }

    Public Void Warning(CTag tag, CStdString& message) override {
        StdString functionName = "-";
        Warning(tag, message, functionName);
    }

    Public Void Error(CTag tag, CStdString& message) override {
        StdString functionName = "-";
        Error(tag, message, functionName);
    }

    Public Void Critical(CTag tag, CStdString& message) override {
        StdString functionName = "-";
        Critical(tag, message, functionName);
    }

    Public Void SetLogLevel(CLogLevel level) override {
        currentLogLevel = level;
    }
    
    Public LogLevel GetLogLevel() const override {
        return currentLogLevel;
    }
    
    Private Static StdString GetLogLevelString(CLogLevel level) {
        switch (level) {
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Verbose: return "VERBOSE";
            case LogLevel::Info: return "INFO";
            case LogLevel::Warning: return "WARNING";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Critical: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
    Private Bool ShouldLog(LogLevel level) const {
        return static_cast<Int>(level) >= static_cast<Int>(currentLogLevel);
    }
    
    Private Void WriteLog(CLogLevel level, CTag /* tag1 */, CTag /* tag2 */, CStdString& message, CStdString& functionName) const {
        StdString output;
        
        // Add function name
        output += "[" + functionName + "] ";
        
        // Add log level
        output += "[" + GetLogLevelString(level) + "] ";
        
        // Add message
        output += message;
        
        // Redirect to log sink
        logSink->WriteLog(output);
    }
};

#endif // LOGGER_INTERNAL_H

