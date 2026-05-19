#ifndef INTERNET_UTIL_H
#define INTERNET_UTIL_H

#include <StandardDefines.h>
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

class InternetUtil {
    
    Public Static Bool IsHostReachable(CStdString host, Int port, Int timeout_ms) {
        esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (!netif || !esp_netif_is_netif_up(netif)) {
            return false;
        }
    
        // Try to connect
        struct addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
    
        struct addrinfo* res;
        if (getaddrinfo(host.c_str(), NULL, &hints, &res) != 0 || res == NULL) {
            return false;
        }
        ((struct sockaddr_in*)res->ai_addr)->sin_port = htons(port);
    
        int sock = socket(res->ai_family, res->ai_socktype, 0);
        if (sock < 0) {
            freeaddrinfo(res);
            return false;
        }
    
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
        bool connected = (connect(sock, res->ai_addr, res->ai_addrlen) == 0);
    
        close(sock);
        freeaddrinfo(res);
        return connected;
    }

};

#endif // INTERNET_UTIL_H