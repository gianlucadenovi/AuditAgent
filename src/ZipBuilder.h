#pragma once

#include <string>
#include <vector>

struct ZipFileItem {
    std::string archiveName;
    std::vector<unsigned char> data;
};

class ZipBuilder {
public:
    static std::vector<unsigned char> BuildZip(const std::vector<ZipFileItem>& files);

private:
    static unsigned int Crc32(const unsigned char* data, size_t size);
    static void Write16(std::vector<unsigned char>& out, unsigned short value);
    static void Write32(std::vector<unsigned char>& out, unsigned int value);
};
