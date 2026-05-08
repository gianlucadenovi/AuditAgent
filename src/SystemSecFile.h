#pragma once

#include <string>

class SystemSecFile {
public:
    static bool ReadDecoded(std::string& decodedText, std::string& error);
    static bool Reset(std::string& error);

private:
    static std::string GetExecutableDirectory();
    static std::string GetFilePath();
    static void InvertBytes(std::string& data);
};

