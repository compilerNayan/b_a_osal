#ifdef ESP_PLATFORM
#ifndef ESPIDF_FILE_MANAGER_INTERNAL_H
#define ESPIDF_FILE_MANAGER_INTERNAL_H

#include <StandardDefines.h>
#include "../../01-interface/IFileManager.h"

#include "esp_spiffs.h"
#include "esp_err.h"
#include <fstream>
#include <sstream>

#include "logger/ILogger.h"

/* @Component */
class EspidfFileManager : public IFileManager {
    Private Bool mounted;

    /* @Autowired */
    Private ILoggerPtr logger;

    Private Void InitSPIFFS() {
        if (mounted) return;

        esp_vfs_spiffs_conf_t conf = {
            .base_path = "/spiffs",
            .partition_label = NULL,
            .max_files = 5,
            .format_if_mount_failed = true
        };

        esp_err_t ret = esp_vfs_spiffs_register(&conf);
        if (ret != ESP_OK) {
            logger->Error(Tag::Untagged,
                "[EspidfFileManager] Failed to mount SPIFFS (" +
                StdString(esp_err_to_name(ret)) + ")");
            mounted = false;
        } else {
            mounted = true;
            logger->Info(Tag::Untagged,
                "[EspidfFileManager] SPIFFS mounted successfully");
        }
    }

    Public EspidfFileManager() {
        mounted = false;
        InitSPIFFS();
    }

    Public Virtual Bool Create(CStdString& filename, CStdString& contents) override {
        InitSPIFFS();
        std::string path = "/spiffs/" + filename;
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            logger->Error(Tag::Untagged,
                "[EspidfFileManager] Failed to create file: " + path);
            return false;
        }
        file << contents;
        file.close();
        logger->Info(Tag::Untagged,
            "[EspidfFileManager] File created: " + path);
        return true;
    }

    Public Virtual StdString Read(CStdString& filename) override {
        InitSPIFFS();
        std::string path = "/spiffs/" + filename;
        std::ifstream file(path);
        if (!file.is_open()) {
            logger->Error(Tag::Untagged,
                "[EspidfFileManager] Failed to read file: " + path);
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        logger->Info(Tag::Untagged,
            "[EspidfFileManager] File read: " + path);
        return buffer.str();
    }

    Public Virtual Bool Update(CStdString& filename, CStdString& contents) override {
        InitSPIFFS();
        std::string path = "/spiffs/" + filename;
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            logger->Error(Tag::Untagged,
                "[EspidfFileManager] Failed to update file: " + path);
            return false;
        }
        file << contents;
        file.close();
        logger->Info(Tag::Untagged,
            "[EspidfFileManager] File updated: " + path);
        return true;
    }

    Public Virtual Bool Delete(CStdString& filename) override {
        InitSPIFFS();
        std::string path = "/spiffs/" + filename;
        if (remove(path.c_str()) != 0) {
            logger->Error(Tag::Untagged,
                "[EspidfFileManager] Failed to delete file: " + path);
            return false;
        }
        logger->Info(Tag::Untagged,
            "[EspidfFileManager] File deleted: " + path);
        return true;
    }

    Public Virtual Bool Append(CStdString& filename, CStdString& contents) override {
        InitSPIFFS();
        std::string path = "/spiffs/" + filename;
        std::ofstream file(path, std::ios::out | std::ios::app);
        if (!file.is_open()) {
            logger->Error(Tag::Untagged,
                "[EspidfFileManager] Failed to append to file: " + path);
            return false;
        }
        file << contents;
        file.close();
        logger->Info(Tag::Untagged,
            "[EspidfFileManager] File appended: " + path);
        return true;
    }
};

#endif // ESPIDF_FILE_MANAGER_INTERNAL_H
#endif // ESP_PLATFORM
