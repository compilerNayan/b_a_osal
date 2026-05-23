#ifdef ESP_PLATFORM
#ifndef HTTP_CLIENT_INTERNAL_H
#define HTTP_CLIENT_INTERNAL_H


#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#include <StandardDefines.h>
#include "logger/ILogger.h"
#include "../../01-interface/01-IHttpClient.h"

/* @Component */
class EspidfHttpClient final : public IHttpClient {

    /* @Autowired */
    Private ILoggerPtr logger;

    // Static event handler inside the class
    Private Static esp_err_t HttpEventHandler(esp_http_client_event_t *evt) {
        if (evt->event_id == HTTP_EVENT_ON_DATA && evt->data_len > 0) {
            StdString* resp = (StdString*) evt->user_data;
            if (resp) {
                resp->append((char*)evt->data, evt->data_len);
            }
        }
        return ESP_OK;
    }

    Public Virtual StdString Request(CStdString& url,
                                    esp_http_client_method_t method,
                                    CStdString& body = "",
                                    CStdString& contentType = "application/json") {
        StdString response;

        esp_http_client_config_t config = {};
        config.url = url.c_str();
        config.method = method;
        config.timeout_ms = 10000;
        config.crt_bundle_attach = esp_crt_bundle_attach;   // enables HTTPS CA bundle
        config.event_handler = HttpEventHandler;            // use static member
        config.user_data = &response;                       // response buffer

        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (!client) {
            logger->Error(Tag::Untagged, "Failed to init HTTP client");
            return "";
        }

        if (!body.empty()) {
            esp_http_client_set_header(client, "Content-Type", contentType.c_str());
            esp_http_client_set_post_field(client, body.c_str(), body.size());
        }

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            Int status = esp_http_client_get_status_code(client);
            Int length = esp_http_client_get_content_length(client);

            logger->Info(Tag::Untagged, "HTTP status=" + std::to_string(status) +
                                        " length=" + std::to_string(length));
        } else {
            logger->Error(Tag::Untagged, "HTTP request failed: " + StdString(esp_err_to_name(err)));
        }

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