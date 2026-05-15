#ifdef ESP_PLATFORM
#ifndef ESPIDF_LOGSINK_H
#define ESPIDF_LOGSINK_H

#include <StandardDefines.h>
#include "../../02-interface/02-ILogSink.h"
//#include "../../02-interface/02-ILogBuffer.h"

/* @Component */
class EspidfLogSink final : public ILogSink {
    /*-- @Autowired */
    //Private ILogBufferPtr logBuffer;

    Public EspidfLogSink() = default;

    Public Virtual ~EspidfLogSink() override = default;

    Public Virtual Void WriteLog(CStdString& message) override {
    }
};

#endif // ESPIDF_LOGSINK_H
#endif // ESP_PLATFORM

