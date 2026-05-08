#pragma once

#include <string>
#include <vector>

class AudioCapture {
public:
    static bool CaptureWav(
        std::vector<unsigned char>& wavBytes,
        std::string& error,
        int seconds = 3
    );
};

