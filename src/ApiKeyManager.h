#pragma once

#include <string>

class ApiKeyManager {
public:
    static bool InitializeFromArgs(int argc, char* argv[], std::string& error);
    static bool ValidateApiKey(const std::string& apiKey);

private:
    static std::string GetExecutableDirectory();
    static std::string GetKeyFilePath();
    static std::string Trim(const std::string& value);
    static std::string Sha256Hex(const std::string& value);

    static bool ReadStoredHash(std::string& hash, std::string& error);
    static bool WriteStoredHash(const std::string& hash, std::string& error);

private:
    static std::string storedHash_;
};
