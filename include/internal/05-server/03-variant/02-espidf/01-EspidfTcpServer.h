#ifdef ESP_PLATFORM
#ifndef ESPIDF_TCP_SERVER_INTERNAL_H
#define ESPIDF_TCP_SERVER_INTERNAL_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <esp_timer.h>
#include <esp_system.h>

#include <StandardDefines.h>
#include "util/Cache.h"  
#include "util/GuidUtil.h"
#include "logger/ILogger.h"

#include "../../02-interface/01-IServer.h"
#include "../../01-type/01-IoTMessage.h"

class EspidfTcpServer final : public IServer {
    Private struct SocketEntry {
        Int sock;
        ~SocketEntry() {
            if (sock >= 0) {
                close(sock); // ensure cleanup on expiry
            }
        }
    };
    
    Private Int serverSock_;
    Private Bool running_;
    Private UInt port_;
    Private ULong receivedMessageCount_;
    Private ULong sentMessageCount_;
    Private Cache<StdString, std::shared_ptr<SocketEntry>> socketCache_;
    
    /* @Autowired */
    Private ILoggerPtr logger;

    Public Explicit EspidfTcpServer()
        : serverSock_(-1),
        running_(false),
        port_(0),
        receivedMessageCount_(0),
        sentMessageCount_(0),
        socketCache_(180000) {
    }
    
    Public Virtual ~EspidfTcpServer() {
        Stop();
    }
    
    Public Virtual Bool Start() override {
        if (running_) return false;
    
        port_ = 8080;
        serverSock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (serverSock_ < 0) {
            logger->Error(Tag::Untagged, "Failed to create server socket");
            return false;
        }
    
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port_);
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
        if (bind(serverSock_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            logger->Error(Tag::Untagged, "Bind failed");
            close(serverSock_);
            return false;
        }
    
        if (listen(serverSock_, 5) < 0) {
            logger->Error(Tag::Untagged, "Listen failed");
            close(serverSock_);
            return false;
        }
    
        running_ = true;
        receivedMessageCount_ = 0;
        sentMessageCount_ = 0;
        logger->Info(Tag::Untagged, "TCP server started on port 8080");
        return true;
    }
    
    Public Virtual Bool Stop() override {
        if (!running_) return false;
        close(serverSock_);
        running_ = false;
        logger->Info(Tag::Untagged, "TCP server stopped");
        return true;
    }
    
    Public Virtual Bool IsRunning() const override {
        return running_;
    }
    
    Public Virtual Bool Restart() override {
        Stop();
        return Start();
    }
    
    Public Virtual Optional<IoTMessage> ReceiveMessage(Optional<StdString> /* path */ = std::nullopt) override {
        if (!running_) return std::nullopt;
    
        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);
        Int clientSock = accept(serverSock_, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSock < 0) {
            logger->Error(Tag::Untagged, "Accept failed");
            return std::nullopt;
        }
    
        char buffer[1024];
        Int len = recv(clientSock, buffer, sizeof(buffer)-1, 0);
        if (len <= 0) {
            logger->Error(Tag::Untagged, "Receive failed");
            close(clientSock);
            return std::nullopt;
        }
        buffer[len] = '\0';
    
        IoTMessage msg;
        msg.guid = GuidUtil::GenerateGuid();
        msg.payload = StdString(buffer);
    
        // Store socket in cache with TTL
        socketCache_.Put(msg.guid, std::make_shared<SocketEntry>(clientSock));
    
        receivedMessageCount_++;
        logger->Info(Tag::Untagged, "Message received, GUID=" + msg.guid);
        return msg;
    }
    
    Public Virtual Bool SendMessage(const IoTMessage& msg, Optional<StdString> /* path */ = std::nullopt) override {
        if (!running_) return false;
    
        auto sockOpt = socketCache_.Get(msg.guid);
        if (!sockOpt.has_value()) {
            logger->Error(Tag::Untagged, "Send failed: GUID expired or not found (" + msg.guid + ")");
            return false;
        }
    
        Int clientSock = sockOpt.value()->sock;
        Int sent = send(clientSock, msg.payload.c_str(), msg.payload.size(), 0);
    
        // Remove from cache → destructor closes socket
        socketCache_.Remove(msg.guid);
    
        if (sent <= 0) {
            logger->Error(Tag::Untagged, "Send failed for GUID=" + msg.guid);
            return false;
        }
    
        sentMessageCount_++;
        logger->Info(Tag::Untagged, "Message sent successfully, GUID=" + msg.guid);
        return true;
    }
};

#endif // ESPIDF_TCP_SERVER_INTERNAL_H
#endif // ESP_PLATFORM
