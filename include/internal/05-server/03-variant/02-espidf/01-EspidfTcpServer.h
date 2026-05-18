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
#include <fcntl.h>   // for fcntl()

#include <StandardDefines.h>
#include "util/Cache.h"  
#include "util/GuidUtil.h"
#include "logger/ILogger.h"

#include "../../02-interface/01-ITcpServer.h"

/* @Component */
class EspidfTcpServer final : public ITcpServer {
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
    
        // Set server socket non-blocking
        int flags = fcntl(serverSock_, F_GETFL, 0);
        fcntl(serverSock_, F_SETFL, flags | O_NONBLOCK);
    
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
    
    Public Virtual Optional<MqttMessage> ReceiveMessage() override {
        if (!running_) return std::nullopt;
    
        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);
        Int clientSock = accept(serverSock_, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSock < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No pending connection
                return std::nullopt;
            }
            logger->Error(Tag::Untagged, "Accept failed");
            return std::nullopt;
        }
    
        // Set client socket non-blocking too
        int flags = fcntl(clientSock, F_GETFL, 0);
        fcntl(clientSock, F_SETFL, flags | O_NONBLOCK);
    
        char buffer[1024];
        Int len = recv(clientSock, buffer, sizeof(buffer)-1, 0);
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data yet
                close(clientSock);
                return std::nullopt;
            }
            logger->Error(Tag::Untagged, "Receive failed");
            close(clientSock);
            return std::nullopt;
        }
        if (len == 0) {
            // Connection closed
            close(clientSock);
            return std::nullopt;
        }
    
        buffer[len] = '\0';
    
        MqttMessage msg;
        msg.guid = GuidUtil::GenerateGuid();
        msg.payload = StdString(buffer);
    
        socketCache_.Put(msg.guid, std::make_shared<SocketEntry>(clientSock));
    
        receivedMessageCount_++;
        logger->Info(Tag::Untagged, "Message received, GUID=" + msg.guid);
        return msg;
    }
    
    Public Virtual Bool SendMessage(const MqttMessage& msg) override {
        if (!running_) return false;
    
        auto sockOpt = socketCache_.Get(msg.guid);
        if (!sockOpt.has_value()) {
            logger->Error(Tag::Untagged, "Send failed: GUID expired or not found (" + msg.guid + ")");
            return false;
        }
    
        Int clientSock = sockOpt.value()->sock;
        Int sent = send(clientSock, msg.payload.c_str(), msg.payload.size(), 0);
    
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Socket not ready → non-blocking fail fast
                logger->Warning(Tag::Untagged, "Send would block for GUID=" + msg.guid);
            } else {
                logger->Error(Tag::Untagged, "Send failed for GUID=" + msg.guid + " errno=" + std::to_string(errno));
            }
            // Remove from cache → destructor closes socket
            socketCache_.Remove(msg.guid);
            return false;
        }
    
        if (sent == 0) {
            // Connection closed by peer
            logger->Warning(Tag::Untagged, "Client closed connection before send, GUID=" + msg.guid);
            socketCache_.Remove(msg.guid);
            return false;
        }
    
        // Success
        sentMessageCount_++;
        logger->Info(Tag::Untagged, "Message sent successfully, GUID=" + msg.guid);
        // Remove from cache → destructor closes socket
        socketCache_.Remove(msg.guid);
        return true;
    }
};

#endif // ESPIDF_TCP_SERVER_INTERNAL_H
#endif // ESP_PLATFORM
