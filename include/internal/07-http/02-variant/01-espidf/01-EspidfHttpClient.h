#ifdef ESP_PLATFORM
#ifndef HTTP_CLIENT_INTERNAL_H
#define HTTP_CLIENT_INTERNAL_H

#include <StandardDefines.h>

#pragma once
#include "esp_http_client.h"
#include "esp_log.h"
#include "StandardDefines.h"

#include "logger/ILogger.h"

#include "../../01-interface/01-IHttpClient.h"

/* @Component */
class EspidfHttpClient final : public IHttpClient {

    /* @Autowired */
    Private ILoggerPtr logger;

    Private Virtual StdString Request(CStdString& url,
                                    esp_http_client_method_t method,
                                    CStdString& body,
                                    CStdString& contentType) 
    {
        StdString response;

        esp_http_client_config_t config = {};
        config.url = url.c_str();
        config.method = method;

        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (!client) {
            logger->Error(Tag::Untagged, "Failed to init HTTP client");
            return "";
        }

        if (!body.empty()) {
            esp_http_client_set_header(client, "Content-Type", contentType.c_str());
            esp_http_client_set_post_field(client, body.c_str(), body.size());
        }

        esp_err_t err = esp_http_client_open(client, body.empty() ? 0 : body.size());
        if (err != ESP_OK) {
            logger->Error(Tag::Untagged, "Failed to open connection: " + std::string(esp_err_to_name(err)));
            esp_http_client_cleanup(client);
            return "";
        }

        Int content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            logger->Error(Tag::Untagged, "HTTP fetch headers failed");
            esp_http_client_cleanup(client);
            return "";
        }

        Char buffer[512];
        Int read_len;
        while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer))) > 0) {
            response.append(buffer, read_len);
        }

        if (read_len < 0) {
            logger->Error(Tag::Untagged, "Error reading response");
            response.clear();
        }

        esp_http_client_close(client);
        esp_http_client_cleanup(client);

        return response;
    }

    Public Virtual StdString Get(CStdString& url) {
        return Request(url, HTTP_METHOD_GET);
    }

    Public Virtual StdString Post(CStdString& url, CStdString& body,
                                CStdString& contentType) {
        return Request(url, HTTP_METHOD_POST, body, contentType);
    }

    Public Virtual StdString Put(CStdString& url, CStdString& body,
                                CStdString& contentType) {
        return Request(url, HTTP_METHOD_PUT, body, contentType);
    }

    Public Virtual StdString Delete(CStdString& url, CStdString& body,
                                    CStdString& contentType) {
        return Request(url, HTTP_METHOD_DELETE, body, contentType);
    }
};

#endif // HTTP_CLIENT_INTERNAL_H
#endif // ESP_PLATFORM