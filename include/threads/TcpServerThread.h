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
        while(!tcpServer_->IsRunning()) {
            tcpServer_->Restart();
            if(tcpServer_->IsRunning()) {
                logger_->Info(Tag::Untagged, "TcpServerThread started");
                while (tcpServer_->IsRunning()) {
                    tcpServer_->ReceiveMessage();
                    tcpServer_->SendMessage();
                }
            } else {
                logger_->Error(Tag::Untagged, "TcpServerThread failed to start");
            }
            Thread::Sleep(2000);
        }
    }
};

#endif // TCPSERVERTHREAD_H
