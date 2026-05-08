#include "SystemSecFile.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <fstream>
#include <sstream>
#include <vector>

std::string SystemSecFile::GetExecutableDirectory() {
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

std::string SystemSecFile::GetFilePath() {
    return GetExecutableDirectory() + "\\system_sec.txc";
}

void SystemSecFile::InvertBytes(std::string& data) {
    for (char& c : data) {
        unsigned char b = static_cast<unsigned char>(c);
        b = static_cast<unsigned char>(~b);
        c = static_cast<char>(b);
    }
}

bool SystemSecFile::ReadDecoded(std::string& decodedText, std::string& error) {
    decodedText.clear();
    error.clear();

    const std::string path = GetFilePath();

    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        error = "Unable to open file: " + path;
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    decodedText = buffer.str();

    file.close();

    InvertBytes(decodedText);

    return true;
}

bool SystemSecFile::Reset(std::string& error) {
    error.clear();

    const std::string path = GetFilePath();

    std::ofstream file(path, std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        error = "Unable to reset file: " + path;
        return false;
    }

    file.close();

    return true;
}
