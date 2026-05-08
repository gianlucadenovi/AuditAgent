#pragma once

#include <string>
#include <vector>

struct BrowserHistoryEntry {
    std::string browser;
    std::string title;
    std::string url;
    std::string lastVisitTime;
    int visitCount = 0;
};

struct BrowserInfo {
    std::string id;
    std::string name;
    std::string historyPath;
    bool available = false;
};

class BrowserHistory {
public:
    static std::vector<BrowserInfo> GetAvailableBrowsers();

    static bool ReadHistory(
        const std::string& browserId,
        std::vector<BrowserHistoryEntry>& entries,
        std::string& error,
        int limit = 300
    );

private:
    static std::string GetEnvVar(const std::string& name);
    static std::string FileExistsPath(const std::string& path);
    static std::string CopyHistoryToTemp(const std::string& sourcePath);
    static std::string ChromeTimeToString(long long chromeTime);
    static std::string FirefoxTimeToString(long long firefoxTime);
    static bool ReadChromiumHistory(
        const BrowserInfo& browser,
        std::vector<BrowserHistoryEntry>& entries,
        std::string& error,
        int limit
    );
    static bool ReadFirefoxHistory(
        const BrowserInfo& browser,
        std::vector<BrowserHistoryEntry>& entries,
        std::string& error,
        int limit
    );
};

