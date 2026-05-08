#pragma once

#include <string>
#include <vector>

class WebcamCapture {
public:
    static bool CaptureJpeg(
        std::vector<unsigned char>& jpegBytes,
        std::string& error,
        int cameraIndex = 0
    );
};
