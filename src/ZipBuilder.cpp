#include "ZipBuilder.h"

struct CentralDirectoryEntry {
    std::string name;
    unsigned int crc;
    unsigned int compressedSize;
    unsigned int uncompressedSize;
    unsigned int localHeaderOffset;
};

void ZipBuilder::Write16(std::vector<unsigned char>& out, unsigned short value) {
    out.push_back(static_cast<unsigned char>(value & 0xff));
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
}

void ZipBuilder::Write32(std::vector<unsigned char>& out, unsigned int value) {
    out.push_back(static_cast<unsigned char>(value & 0xff));
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    out.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    out.push_back(static_cast<unsigned char>((value >> 24) & 0xff));
}

unsigned int ZipBuilder::Crc32(const unsigned char* data, size_t size) {
    unsigned int crc = 0xFFFFFFFFu;

    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];

        for (int j = 0; j < 8; ++j) {
            unsigned int mask = (crc & 1u) ? 0xFFFFFFFFu : 0u;
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return ~crc;
}

std::vector<unsigned char> ZipBuilder::BuildZip(const std::vector<ZipFileItem>& files) {
    std::vector<unsigned char> out;
    std::vector<CentralDirectoryEntry> central;

    for (const auto& file : files) {
        CentralDirectoryEntry entry;
        entry.name = file.archiveName;
        entry.crc = Crc32(file.data.data(), file.data.size());
        entry.compressedSize = static_cast<unsigned int>(file.data.size());
        entry.uncompressedSize = static_cast<unsigned int>(file.data.size());
        entry.localHeaderOffset = static_cast<unsigned int>(out.size());

        Write32(out, 0x04034b50);
        Write16(out, 20);
        Write16(out, 0);
        Write16(out, 0);
        Write16(out, 0);
        Write16(out, 0);
        Write32(out, entry.crc);
        Write32(out, entry.compressedSize);
        Write32(out, entry.uncompressedSize);
        Write16(out, static_cast<unsigned short>(entry.name.size()));
        Write16(out, 0);

        out.insert(out.end(), entry.name.begin(), entry.name.end());
        out.insert(out.end(), file.data.begin(), file.data.end());

        central.push_back(entry);
    }

    unsigned int centralStart = static_cast<unsigned int>(out.size());

    for (const auto& entry : central) {
        Write32(out, 0x02014b50);
        Write16(out, 20);
        Write16(out, 20);
        Write16(out, 0);
        Write16(out, 0);
        Write16(out, 0);
        Write16(out, 0);
        Write32(out, entry.crc);
        Write32(out, entry.compressedSize);
        Write32(out, entry.uncompressedSize);
        Write16(out, static_cast<unsigned short>(entry.name.size()));
        Write16(out, 0);
        Write16(out, 0);
        Write16(out, 0);
        Write16(out, 0);
        Write32(out, 0);
        Write32(out, entry.localHeaderOffset);

        out.insert(out.end(), entry.name.begin(), entry.name.end());
    }

    unsigned int centralSize = static_cast<unsigned int>(out.size()) - centralStart;

    Write32(out, 0x06054b50);
    Write16(out, 0);
    Write16(out, 0);
    Write16(out, static_cast<unsigned short>(central.size()));
    Write16(out, static_cast<unsigned short>(central.size()));
    Write32(out, centralSize);
    Write32(out, centralStart);
    Write16(out, 0);

    return out;
}
