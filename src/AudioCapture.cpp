#include "AudioCapture.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <mmreg.h>
#include <wrl/client.h>

#include <vector>
#include <string>
#include <sstream>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

using Microsoft::WRL::ComPtr;

static std::string HResultToString(HRESULT hr, const std::string& message) {
    std::ostringstream out;
    out << message << " HRESULT=0x" << std::hex << static_cast<unsigned long>(hr);
    return out.str();
}

static int ClampInt(int value, int minValue, int maxValue) {
    if (value < minValue) {
        return minValue;
    }

    if (value > maxValue) {
        return maxValue;
    }

    return value;
}

static void WriteBytes(std::vector<unsigned char>& out, const void* data, size_t size) {
    const unsigned char* p = static_cast<const unsigned char*>(data);
    out.insert(out.end(), p, p + size);
}

static void WriteU16(std::vector<unsigned char>& out, unsigned short value) {
    out.push_back(static_cast<unsigned char>(value & 0xff));
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
}

static void WriteU32(std::vector<unsigned char>& out, unsigned int value) {
    out.push_back(static_cast<unsigned char>(value & 0xff));
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    out.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    out.push_back(static_cast<unsigned char>((value >> 24) & 0xff));
}

static void BuildWav(
    const WAVEFORMATEX* format,
    const std::vector<unsigned char>& pcmData,
    std::vector<unsigned char>& wavBytes
) {
    wavBytes.clear();

    unsigned int fmtSize = 16;

    if (format->wFormatTag != WAVE_FORMAT_PCM) {
        fmtSize = static_cast<unsigned int>(sizeof(WAVEFORMATEX) + format->cbSize);
    }

    unsigned int riffSize =
        4 +
        8 + fmtSize +
        8 + static_cast<unsigned int>(pcmData.size());

    WriteBytes(wavBytes, "RIFF", 4);
    WriteU32(wavBytes, riffSize);
    WriteBytes(wavBytes, "WAVE", 4);

    WriteBytes(wavBytes, "fmt ", 4);
    WriteU32(wavBytes, fmtSize);
    WriteBytes(wavBytes, format, fmtSize);

    WriteBytes(wavBytes, "data", 4);
    WriteU32(wavBytes, static_cast<unsigned int>(pcmData.size()));

    if (!pcmData.empty()) {
        WriteBytes(wavBytes, pcmData.data(), pcmData.size());
    }
}

bool AudioCapture::CaptureWav(
    std::vector<unsigned char>& wavBytes,
    std::string& error,
    int seconds
) {
    wavBytes.clear();
    error.clear();

    seconds = ClampInt(seconds, 1, 30);

    HRESULT comHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    bool shouldUninitializeCom = false;

    if (SUCCEEDED(comHr)) {
        shouldUninitializeCom = true;
    }
    else if (comHr != RPC_E_CHANGED_MODE) {
        error = HResultToString(comHr, "Unable to initialize COM.");
        return false;
    }

    ComPtr<IMMDeviceEnumerator> enumerator;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&enumerator)
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to create audio device enumerator.");

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    ComPtr<IMMDevice> device;

    hr = enumerator->GetDefaultAudioEndpoint(
        eCapture,
        eConsole,
        &device
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to get default microphone.");

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    ComPtr<IAudioClient> audioClient;

    hr = device->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        nullptr,
        reinterpret_cast<void**>(audioClient.GetAddressOf())
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to activate microphone audio client.");

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    WAVEFORMATEX* mixFormat = nullptr;

    hr = audioClient->GetMixFormat(&mixFormat);

    if (FAILED(hr) || !mixFormat) {
        error = HResultToString(hr, "Unable to get microphone audio format.");

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    REFERENCE_TIME bufferDuration = static_cast<REFERENCE_TIME>(seconds) * 10000000LL;

    hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        bufferDuration,
        0,
        mixFormat,
        nullptr
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to initialize microphone capture.");
        CoTaskMemFree(mixFormat);

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    ComPtr<IAudioCaptureClient> captureClient;

    hr = audioClient->GetService(
        IID_PPV_ARGS(&captureClient)
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to get microphone capture client.");
        CoTaskMemFree(mixFormat);

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    hr = audioClient->Start();

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to start microphone capture.");
        CoTaskMemFree(mixFormat);

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    std::vector<unsigned char> audioData;

    ULONGLONG startTime = GetTickCount64();
    ULONGLONG endTime = startTime + static_cast<ULONGLONG>(seconds) * 1000ULL;

    while (GetTickCount64() < endTime) {
        UINT32 packetLength = 0;

        hr = captureClient->GetNextPacketSize(&packetLength);

        if (FAILED(hr)) {
            error = HResultToString(hr, "Unable to read microphone packet size.");
            audioClient->Stop();
            CoTaskMemFree(mixFormat);

            if (shouldUninitializeCom) {
                CoUninitialize();
            }

            return false;
        }

        while (packetLength > 0) {
            BYTE* data = nullptr;
            UINT32 framesAvailable = 0;
            DWORD flags = 0;

            hr = captureClient->GetBuffer(
                &data,
                &framesAvailable,
                &flags,
                nullptr,
                nullptr
            );

            if (FAILED(hr)) {
                error = HResultToString(hr, "Unable to read microphone buffer.");
                audioClient->Stop();
                CoTaskMemFree(mixFormat);

                if (shouldUninitializeCom) {
                    CoUninitialize();
                }

                return false;
            }

            size_t bytesToCopy =
                static_cast<size_t>(framesAvailable) *
                static_cast<size_t>(mixFormat->nBlockAlign);

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                size_t oldSize = audioData.size();
                audioData.resize(oldSize + bytesToCopy, 0);
            }
            else if (data && bytesToCopy > 0) {
                audioData.insert(audioData.end(), data, data + bytesToCopy);
            }

            hr = captureClient->ReleaseBuffer(framesAvailable);

            if (FAILED(hr)) {
                error = HResultToString(hr, "Unable to release microphone buffer.");
                audioClient->Stop();
                CoTaskMemFree(mixFormat);

                if (shouldUninitializeCom) {
                    CoUninitialize();
                }

                return false;
            }

            hr = captureClient->GetNextPacketSize(&packetLength);

            if (FAILED(hr)) {
                error = HResultToString(hr, "Unable to read microphone packet size.");
                audioClient->Stop();
                CoTaskMemFree(mixFormat);

                if (shouldUninitializeCom) {
                    CoUninitialize();
                }

                return false;
            }
        }

        Sleep(10);
    }

    audioClient->Stop();

    if (audioData.empty()) {
        error = "No microphone audio was captured.";
        CoTaskMemFree(mixFormat);

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    BuildWav(mixFormat, audioData, wavBytes);

    CoTaskMemFree(mixFormat);

    if (shouldUninitializeCom) {
        CoUninitialize();
    }

    return true;
}
