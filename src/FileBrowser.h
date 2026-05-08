#pragma once

#include <string>
#include <vector>

struct FileEntry {
    std::string name;
    std::string path;
    bool isDirectory;
    unsigned long long size;
};

class FileBrowser {
public:
    explicit FileBrowser(std::string allowedRoot);

    bool ListDirectory(
        const std::string& requestedPath,
        std::vector<FileEntry>& entries,
        std::string& resolvedPath,
        std::string& error
    ) const;

    bool ResolveAllowedPath(
        const std::string& requestedPath,
        std::string& resolvedPath,
        std::string& error
    ) const;

    bool IsRegularFile(
        const std::string& requestedPath,
        std::string& resolvedPath,
        std::string& error
    ) const;

    const std::string& GetAllowedRoot() const;

private:
    std::string allowedRoot_;

    static std::string NormalizePath(const std::string& path);
    bool IsPathAllowed(const std::string& path) const;
};

