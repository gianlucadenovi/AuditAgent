#include "FileBrowser.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <utility>
#include <cctype>

FileBrowser::FileBrowser(std::string allowedRoot)
    : allowedRoot_(NormalizePath(std::move(allowedRoot))) {
}

const std::string& FileBrowser::GetAllowedRoot() const {
    return allowedRoot_;
}

std::string FileBrowser::NormalizePath(const std::string& path) {
    char fullPath[MAX_PATH];

    DWORD result = GetFullPathNameA(
        path.c_str(),
        MAX_PATH,
        fullPath,
        nullptr
    );

    if (result == 0 || result >= MAX_PATH) {
        return path;
    }

    std::string normalized(fullPath);

    while (!normalized.empty() &&
        (normalized.back() == '\\' || normalized.back() == '/')) {
        normalized.pop_back();
    }

    std::replace(normalized.begin(), normalized.end(), '/', '\\');

    return normalized;
}

bool FileBrowser::IsPathAllowed(const std::string& path) const {
    std::string normalized = NormalizePath(path);

    if (normalized.size() < allowedRoot_.size()) {
        return false;
    }

    std::string prefix = normalized.substr(0, allowedRoot_.size());

    std::string a = prefix;
    std::string b = allowedRoot_;

    std::transform(a.begin(), a.end(), a.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
        });

    std::transform(b.begin(), b.end(), b.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
        });

    if (a != b) {
        return false;
    }

    if (normalized.size() == allowedRoot_.size()) {
        return true;
    }

    if (allowedRoot_.size() == 3 && allowedRoot_[1] == ':' && allowedRoot_[2] == '\\') {
        return true;
    }

    return normalized[allowedRoot_.size()] == '\\';
}

bool FileBrowser::ResolveAllowedPath(
    const std::string& requestedPath,
    std::string& resolvedPath,
    std::string& error
) const {
    resolvedPath = NormalizePath(requestedPath.empty() ? allowedRoot_ : requestedPath);

    if (!IsPathAllowed(resolvedPath)) {
        error = "Path is outside the allowed root.";
        return false;
    }

    return true;
}

bool FileBrowser::IsRegularFile(
    const std::string& requestedPath,
    std::string& resolvedPath,
    std::string& error
) const {
    if (!ResolveAllowedPath(requestedPath, resolvedPath, error)) {
        return false;
    }

    DWORD attrs = GetFileAttributesA(resolvedPath.c_str());

    if (attrs == INVALID_FILE_ATTRIBUTES) {
        error = "File does not exist.";
        return false;
    }

    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        error = "Path is a directory, not a file.";
        return false;
    }

    return true;
}

bool FileBrowser::ListDirectory(
    const std::string& requestedPath,
    std::vector<FileEntry>& entries,
    std::string& resolvedPath,
    std::string& error
) const {
    if (!ResolveAllowedPath(requestedPath, resolvedPath, error)) {
        return false;
    }

    DWORD attrs = GetFileAttributesA(resolvedPath.c_str());

    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        error = "Path is not a valid directory.";
        return false;
    }

    std::string searchPath = resolvedPath + "\\*";

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        error = "Unable to list directory.";
        return false;
    }

    entries.clear();

    do {
        std::string name = findData.cFileName;

        if (name == "." || name == "..") {
            continue;
        }

        FileEntry entry;
        entry.name = name;
        entry.path = resolvedPath + "\\" + name;
        entry.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        ULARGE_INTEGER size{};
        size.HighPart = findData.nFileSizeHigh;
        size.LowPart = findData.nFileSizeLow;
        entry.size = size.QuadPart;

        entries.push_back(entry);

    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);

    std::sort(entries.begin(), entries.end(), [](const FileEntry& a, const FileEntry& b) {
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory > b.isDirectory;
        }

        std::string an = a.name;
        std::string bn = b.name;

        std::transform(an.begin(), an.end(), an.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
            });

        std::transform(bn.begin(), bn.end(), bn.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
            });

        return an < bn;
        });

    return true;
}
