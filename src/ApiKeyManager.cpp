#include "ApiKeyManager.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <bcrypt.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cctype>

#pragma comment(lib, "bcrypt.lib")

std::string ApiKeyManager::storedHash_;

std::string ApiKeyManager::GetExecutableDirectory() {
    char path[MAX_PATH];

    DWORD len = GetModuleFileNameA(
        nullptr,
        path,
        MAX_PATH
    );

    if (len == 0 || len >= MAX_PATH) {
        return ".";
    }

    std::string fullPath(path);
    size_t pos = fullPath.find_last_of("\\/");

    if (pos == std::string::npos) {
        return ".";
    }

    return fullPath.substr(0, pos);
}

std::string ApiKeyManager::GetKeyFilePath() {
    return GetExecutableDirectory() + "\\system_ak.txc";
}

std::string ApiKeyManager::Trim(const std::string& value) {
    size_t start = 0;
    size_t end = value.size();

    while (start < end && std::isspace(static_cast<unsigned char>(value[start]))) {
        start++;
    }

    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        end--;
    }

    return value.substr(start, end - start);
}

std::string ApiKeyManager::Sha256Hex(const std::string& value) {
    BCRYPT_ALG_HANDLE algHandle = nullptr;
    BCRYPT_HASH_HANDLE hashHandle = nullptr;

    DWORD objectLength = 0;
    DWORD dataLength = 0;
    DWORD hashLength = 0;

    std::vector<unsigned char> hashObject;
    std::vector<unsigned char> hash;

    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &algHandle,
        BCRYPT_SHA256_ALGORITHM,
        nullptr,
        0
    );

    if (status < 0) {
        return "";
    }

    status = BCryptGetProperty(
        algHandle,
        BCRYPT_OBJECT_LENGTH,
        reinterpret_cast<PUCHAR>(&objectLength),
        sizeof(DWORD),
        &dataLength,
        0
    );

    if (status < 0) {
        BCryptCloseAlgorithmProvider(algHandle, 0);
        return "";
    }

    status = BCryptGetProperty(
        algHandle,
        BCRYPT_HASH_LENGTH,
        reinterpret_cast<PUCHAR>(&hashLength),
        sizeof(DWORD),
        &dataLength,
        0
    );

    if (status < 0) {
        BCryptCloseAlgorithmProvider(algHandle, 0);
        return "";
    }

    hashObject.resize(objectLength);
    hash.resize(hashLength);

    status = BCryptCreateHash(
        algHandle,
        &hashHandle,
        hashObject.data(),
        objectLength,
        nullptr,
        0,
        0
    );

    if (status < 0) {
        BCryptCloseAlgorithmProvider(algHandle, 0);
        return "";
    }

    status = BCryptHashData(
        hashHandle,
        reinterpret_cast<PUCHAR>(const_cast<char*>(value.data())),
        static_cast<ULONG>(value.size()),
        0
    );

    if (status < 0) {
        BCryptDestroyHash(hashHandle);
        BCryptCloseAlgorithmProvider(algHandle, 0);
        return "";
    }

    status = BCryptFinishHash(
        hashHandle,
        hash.data(),
        hashLength,
        0
    );

    BCryptDestroyHash(hashHandle);
    BCryptCloseAlgorithmProvider(algHandle, 0);

    if (status < 0) {
        return "";
    }

    std::ostringstream out;

    for (unsigned char b : hash) {
        out << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }

    return out.str();
}

bool ApiKeyManager::ReadStoredHash(std::string& hash, std::string& error) {
    hash.clear();
    error.clear();

    std::ifstream file(GetKeyFilePath(), std::ios::binary);

    if (!file.is_open()) {
        error = "system_ak.txc not found.";
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    hash = Trim(buffer.str());

    if (hash.empty()) {
        error = "system_ak.txc is empty.";
        return false;
    }

    storedHash_ = hash;
    return true;
}

bool ApiKeyManager::WriteStoredHash(const std::string& hash, std::string& error) {
    error.clear();

    std::ofstream file(GetKeyFilePath(), std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        error = "Unable to create system_ak.txc.";
        return false;
    }

    file << hash;
    file.close();

    return true;
}

bool ApiKeyManager::InitializeFromArgs(int argc, char* argv[], std::string& error) {
    error.clear();

    std::string existingHash;

    if (ReadStoredHash(existingHash, error)) {
        storedHash_ = existingHash;
        return true;
    }

    std::string apiKey;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.rfind("k=", 0) == 0) {
            apiKey = arg.substr(2);
            break;
        }
    }

    if (apiKey.empty()) {
        error = "system_ak.txc does not exist and no k=[apikey] argument was provided.";
        return false;
    }

    std::string hash = Sha256Hex(apiKey);

    if (hash.empty()) {
        error = "Unable to hash API key.";
        return false;
    }

    if (!WriteStoredHash(hash, error)) {
        return false;
    }

    storedHash_ = hash;
    return true;
}

bool ApiKeyManager::ValidateApiKey(const std::string& apiKey) {
    if (apiKey.empty() || storedHash_.empty()) {
        return false;
    }

    std::string hash = Sha256Hex(apiKey);

    if (hash.empty()) {
        return false;
    }

    return hash == storedHash_;
}
