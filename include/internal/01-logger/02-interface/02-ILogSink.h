#ifndef ILOGSINK_H
#define ILOGSINK_H

#include <StandardDefines.h>

DefineStandardPointers(ILogSink)
class ILogSink {
    Public Virtual ~ILogSink() = default;
    
    // Core method to write log messages
    Public Virtual Void WriteLog(CStdString& message) = 0;
};

#endif // ILOGSINK_H

