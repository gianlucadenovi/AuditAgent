#pragma once

#include <atomic>
#include <string>
#include <vector>

#include "httplib.h"
#include "FileBrowser.h"

#include <json/json.h>

class AuditWebServer {
public:
    AuditWebServer(
        std::string host,
        int port,
        std::string allowedRoot
    );

    bool Start();
    void Stop();

private:
    std::string host_;
    int port_;

    httplib::Server server_;
    std::atomic<bool> running_;

    FileBrowser fileBrowser_;

    void RegisterRoutes();
    bool IsAuthorized(const httplib::Request& req) const;

    static bool ParseJsonBody(const httplib::Request& req, Json::Value& root, std::string& error);

    static std::string LoginHtml();
    static std::string JsonEscape(const std::string& value);
    static std::string MimeTypeForPath(const std::string& path);
    static std::string FileNameFromPath(const std::string& path);
    static bool ReadFileBytes(const std::string& path, std::vector<unsigned char>& data);
    static std::vector<std::string> ExtractJsonStringArray(const Json::Value& root, const std::string& key);
};