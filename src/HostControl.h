#pragma once

#include <string>

class HostControl {
public:
    static bool ForceReboot(std::string& error);
};

