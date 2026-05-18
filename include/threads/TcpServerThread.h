#ifndef TCPSERVERTHREAD_H
#define TCPSERVERTHREAD_H

#include <StandardDefines.h>
#include "thread.h"
#include "internal/05-server/02-interface/01-ITcpServer.h"
#include "logger/ILogger.h"
#include "threading/IRunnable.h"

class TcpServerThread final : public IRunnable {
    /* @Autowired */
    Private ITcpServerPtr tcpServer_;

    /* @Autowired */
    Private ILoggerPtr logger_;

    Public TcpServerThread() = default;

    Public Virtual ~TcpServerThread() = default;

    // IRunnable implementation
    Public Virtual Void Run() override {
        logger_->Info(Tag::Untagged, "TcpServerThread started");
        tcpServer_->Restart();
        while (tcpServer_ && tcpServer_->IsRunning()) {
            tcpServer_->ReceiveMessage();
            tcpServer_->SendMessage();
            Thread::Sleep(400); // sleep 400 ms
        }
        logger_->Info(Tag::Untagged, "TcpServerThread stopped");
    }
};

#endif // TCPSERVERTHREAD_H
