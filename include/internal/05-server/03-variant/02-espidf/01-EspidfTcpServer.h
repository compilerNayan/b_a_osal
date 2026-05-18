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
#include <deque>
#include <mutex>

#include <StandardDefines.h>
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

    // Buffers
    Private StdDeque<MqttMessage> receiveBuffer_;
    Private StdDeque<MqttMessage> sendBuffer_;

    // Mutexes for coarse-grained locking
    Private mutable std::mutex receiveMutex_;
    Private mutable std::mutex sendMutex_;

    /* @Autowired */
    Private ILoggerPtr logger;

    Public Explicit EspidfTcpServer()
        : serverSock_(-1),
          running_(false),
          port_(0),
          receivedMessageCount_(0),
          sentMessageCount_(0) {
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

    // New design: ReceiveMessage just buffers internally
    Public Virtual Void ReceiveMessage() override {
        if (!running_) return;

        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);
        Int clientSock = accept(serverSock_, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSock < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return; // No pending connection
            }
            logger->Error(Tag::Untagged, "Accept failed");
            return;
        }

        // Set client socket non-blocking too
        int flags = fcntl(clientSock, F_GETFL, 0);
        fcntl(clientSock, F_SETFL, flags | O_NONBLOCK);

        char buffer[1024];
        Int len = recv(clientSock, buffer, sizeof(buffer)-1, 0);
        if (len <= 0) {
            close(clientSock);
            return;
        }

        buffer[len] = '\0';

        MqttMessage msg;
        msg.guid = GuidUtil::GenerateGuid();
        msg.payload = StdString(buffer);

        {
            std::lock_guard<std::mutex> lock(receiveMutex_);
            receiveBuffer_.push_back(msg);
        }

        receivedMessageCount_++;
        logger->Info(Tag::Untagged, "Message buffered, GUID=" + msg.guid);
    }

    // New design: SendMessage drains one from send buffer if available
    Public Virtual Void SendMessage() override {
        if (!running_) return;

        MqttMessage msg;
        {
            std::lock_guard<std::mutex> lock(sendMutex_);
            if (sendBuffer_.empty()) return;
            msg = sendBuffer_.front();
            sendBuffer_.pop_front();
        }

        // For simplicity, assume one active client socket per message
        // In real design, you'd track sockets per client
        // Here we just try to send to the last accepted socket
        Int clientSock = serverSock_; // placeholder, adapt as needed
        Int sent = send(clientSock, msg.payload.c_str(), msg.payload.size(), 0);

        if (sent <= 0) {
            logger->Warning(Tag::Untagged, "Send failed or would block, GUID=" + msg.guid);
            return;
        }

        sentMessageCount_++;
        logger->Info(Tag::Untagged, "Message sent successfully, GUID=" + msg.guid);
    }

    // Application-facing buffer access
    Public Virtual Optional<MqttMessage> GetNextReceivedMessage() override {
        std::lock_guard<std::mutex> lock(receiveMutex_);
        if (receiveBuffer_.empty()) return std::nullopt;

        auto msg = receiveBuffer_.front();
        receiveBuffer_.pop_front();
        return msg;
    }

    Public Virtual Void QueueMessageToSend(const MqttMessage& msg) override {
        std::lock_guard<std::mutex> lock(sendMutex_);
        sendBuffer_.push_back(msg);
    }

    Public Virtual Size GetPendingReceivedCount() const override {
        std::lock_guard<std::mutex> lock(receiveMutex_);
        return receiveBuffer_.size();
    }

    Public Virtual Size GetPendingSendCount() const override {
        std::lock_guard<std::mutex> lock(sendMutex_);
        return sendBuffer_.size();
    }
};

#endif // ESPIDF_TCP_SERVER_INTERNAL_H
#endif // ESP_PLATFORM
