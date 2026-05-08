#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <cstring>

#include "ScreenCapture.h"

std::vector<unsigned char> ScreenCapture::CaptureScreenBmp() {
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    HDC screenDC = GetDC(nullptr);
    HDC memoryDC = CreateCompatibleDC(screenDC);

    HBITMAP bitmap = CreateCompatibleBitmap(screenDC, width, height);
    HGDIOBJ oldBitmap = SelectObject(memoryDC, bitmap);

    BitBlt(memoryDC, 0, 0, width, height, screenDC, x, y, SRCCOPY | CAPTUREBLT);

    BITMAPINFOHEADER bi{};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    int rowSize = ((width * 3 + 3) / 4) * 4;
    int imageSize = rowSize * height;

    std::vector<unsigned char> pixels(imageSize);

    GetDIBits(
        memoryDC,
        bitmap,
        0,
        height,
        pixels.data(),
        reinterpret_cast<BITMAPINFO*>(&bi),
        DIB_RGB_COLORS
    );

    BITMAPFILEHEADER bf{};
    bf.bfType = 0x4D42;
    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bf.bfSize = bf.bfOffBits + imageSize;

    std::vector<unsigned char> bmp;
    bmp.resize(bf.bfSize);

    std::memcpy(bmp.data(), &bf, sizeof(BITMAPFILEHEADER));
    std::memcpy(bmp.data() + sizeof(BITMAPFILEHEADER), &bi, sizeof(BITMAPINFOHEADER));
    std::memcpy(bmp.data() + bf.bfOffBits, pixels.data(), imageSize);

    SelectObject(memoryDC, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memoryDC);
    ReleaseDC(nullptr, screenDC);

    return bmp;
}