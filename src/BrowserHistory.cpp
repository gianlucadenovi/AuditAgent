#include "BrowserHistory.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

#include "sqlite3.h"

std::string BrowserHistory::GetEnvVar(const std::string& name) {
    char buffer[32767];
    DWORD len = GetEnvironmentVariableA(name.c_str(), buffer, sizeof(buffer));

    if (len == 0 || len >= sizeof(buffer)) {
        return "";
    }

    return std::string(buffer, len);
}

std::string BrowserHistory::FileExistsPath(const std::string& path) {
    DWORD attrs = GetFileAttributesA(path.c_str());

    if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        return "";
    }

    return path;
}

std::vector<BrowserInfo> BrowserHistory::GetAvailableBrowsers() {
    std::vector<BrowserInfo> browsers;

    std::string localAppData = GetEnvVar("LOCALAPPDATA");
    std::string appData = GetEnvVar("APPDATA");

    BrowserInfo chrome;
    chrome.id = "chrome";
    chrome.name = "Chrome";
    chrome.historyPath = FileExistsPath(localAppData + "\\Google\\Chrome\\User Data\\Default\\History");
    chrome.available = !chrome.historyPath.empty();
    browsers.push_back(chrome);

    BrowserInfo edge;
    edge.id = "edge";
    edge.name = "Edge";
    edge.historyPath = FileExistsPath(localAppData + "\\Microsoft\\Edge\\User Data\\Default\\History");
    edge.available = !edge.historyPath.empty();
    browsers.push_back(edge);

    BrowserInfo firefox;
    firefox.id = "firefox";
    firefox.name = "Firefox";

    std::string profilesDir = appData + "\\Mozilla\\Firefox\\Profiles";
    if (std::filesystem::exists(profilesDir)) {
        for (const auto& dir : std::filesystem::directory_iterator(profilesDir)) {
            if (!dir.is_directory()) {
                continue;
            }

            std::string candidate = dir.path().string() + "\\places.sqlite";

            if (!FileExistsPath(candidate).empty()) {
                firefox.historyPath = candidate;
                firefox.available = true;
                break;
            }
        }
    }

    browsers.push_back(firefox);

    return browsers;
}

std::string BrowserHistory::CopyHistoryToTemp(const std::string& sourcePath) {
    std::string tempDir = GetEnvVar("TEMP");

    if (tempDir.empty()) {
        tempDir = ".";
    }

    std::string tempPath = tempDir + "\\auditagent_history_" + std::to_string(GetTickCount64()) + ".sqlite";

    try {
        std::filesystem::copy_file(
            sourcePath,
            tempPath,
            std::filesystem::copy_options::overwrite_existing
        );
    }
    catch (...) {
        return "";
    }

    return tempPath;
}

std::string BrowserHistory::ChromeTimeToString(long long chromeTime) {
    // Chrome timestamp: microseconds since 1601-01-01 UTC.
    // Unix timestamp: seconds since 1970-01-01 UTC.
    long long unixSeconds = (chromeTime / 1000000LL) - 11644473600LL;

    std::time_t t = static_cast<std::time_t>(unixSeconds);
    std::tm localTm{};

    localtime_s(&localTm, &t);

    std::ostringstream out;
    out << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

std::string BrowserHistory::FirefoxTimeToString(long long firefoxTime) {
    // Firefox moz_places.last_visit_date: microseconds since Unix epoch.
    long long unixSeconds = firefoxTime / 1000000LL;

    std::time_t t = static_cast<std::time_t>(unixSeconds);
    std::tm localTm{};

    localtime_s(&localTm, &t);

    std::ostringstream out;
    out << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

bool BrowserHistory::ReadHistory(
    const std::string& browserId,
    std::vector<BrowserHistoryEntry>& entries,
    std::string& error,
    int limit
) {
    auto browsers = GetAvailableBrowsers();

    for (const auto& browser : browsers) {
        if (browser.id == browserId) {
            if (!browser.available) {
                error = browser.name + " history database not found.";
                return false;
            }

            if (browser.id == "chrome" || browser.id == "edge") {
                return ReadChromiumHistory(browser, entries, error, limit);
            }

            if (browser.id == "firefox") {
                return ReadFirefoxHistory(browser, entries, error, limit);
            }
        }
    }

    error = "Unknown browser: " + browserId;
    return false;
}

bool BrowserHistory::ReadChromiumHistory(
    const BrowserInfo& browser,
    std::vector<BrowserHistoryEntry>& entries,
    std::string& error,
    int limit
) {
    std::string tempPath = CopyHistoryToTemp(browser.historyPath);

    if (tempPath.empty()) {
        error = "Unable to copy browser history database. Browser may be locking it.";
        return false;
    }

    sqlite3* db = nullptr;

    if (sqlite3_open_v2(tempPath.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        error = "Unable to open copied history database.";
        std::filesystem::remove(tempPath);
        return false;
    }

    std::string sql =
        "SELECT url, title, visit_count, last_visit_time "
        "FROM urls "
        "ORDER BY last_visit_time DESC "
        "LIMIT ?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        error = "Unable to prepare Chromium history query.";
        sqlite3_close(db);
        std::filesystem::remove(tempPath);
        return false;
    }

    sqlite3_bind_int(stmt, 1, limit);

    entries.clear();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        BrowserHistoryEntry e;
        e.browser = browser.name;

        const unsigned char* url = sqlite3_column_text(stmt, 0);
        const unsigned char* title = sqlite3_column_text(stmt, 1);

        e.url = url ? reinterpret_cast<const char*>(url) : "";
        e.title = title ? reinterpret_cast<const char*>(title) : "";
        e.visitCount = sqlite3_column_int(stmt, 2);

        long long lastVisit = sqlite3_column_int64(stmt, 3);
        e.lastVisitTime = ChromeTimeToString(lastVisit);

        entries.push_back(e);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    std::filesystem::remove(tempPath);

    return true;
}

bool BrowserHistory::ReadFirefoxHistory(
    const BrowserInfo& browser,
    std::vector<BrowserHistoryEntry>& entries,
    std::string& error,
    int limit
) {
    std::string tempPath = CopyHistoryToTemp(browser.historyPath);

    if (tempPath.empty()) {
        error = "Unable to copy Firefox history database.";
        return false;
    }

    sqlite3* db = nullptr;

    if (sqlite3_open_v2(tempPath.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        error = "Unable to open copied Firefox history database.";
        std::filesystem::remove(tempPath);
        return false;
    }

    std::string sql =
        "SELECT url, title, visit_count, last_visit_date "
        "FROM moz_places "
        "WHERE last_visit_date IS NOT NULL "
        "ORDER BY last_visit_date DESC "
        "LIMIT ?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        error = "Unable to prepare Firefox history query.";
        sqlite3_close(db);
        std::filesystem::remove(tempPath);
        return false;
    }

    sqlite3_bind_int(stmt, 1, limit);

    entries.clear();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        BrowserHistoryEntry e;
        e.browser = browser.name;

        const unsigned char* url = sqlite3_column_text(stmt, 0);
        const unsigned char* title = sqlite3_column_text(stmt, 1);

        e.url = url ? reinterpret_cast<const char*>(url) : "";
        e.title = title ? reinterpret_cast<const char*>(title) : "";
        e.visitCount = sqlite3_column_int(stmt, 2);

        long long lastVisit = sqlite3_column_int64(stmt, 3);
        e.lastVisitTime = FirefoxTimeToString(lastVisit);

        entries.push_back(e);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    std::filesystem::remove(tempPath);

    return true;
}
