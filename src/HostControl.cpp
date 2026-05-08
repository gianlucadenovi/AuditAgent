#include "HostControl.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

bool HostControl::ForceReboot(std::string& error) {
    error.clear();

    std::string command = "C:\\Windows\\System32\\shutdown.exe /r /f /t 0";

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};

    si.cb = sizeof(si);

    char cmdLine[512];
    strncpy_s(cmdLine, command.c_str(), _TRUNCATE);

    BOOL ok = CreateProcessA(
        nullptr,
        cmdLine,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (!ok) {
        DWORD err = GetLastError();
        error = "Failed to start reboot command. Windows error: " + std::to_string(err);
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}
