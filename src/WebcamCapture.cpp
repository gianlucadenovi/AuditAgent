#include "WebcamCapture.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <string>
#include <vector>
#include <sstream>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")

using Microsoft::WRL::ComPtr;

static std::string HResultToString(HRESULT hr, const std::string& message) {
    std::ostringstream out;
    out << message << " HRESULT=0x" << std::hex << static_cast<unsigned long>(hr);
    return out.str();
}

static bool EncodeRgb32ToJpeg(
    const unsigned char* pixels,
    unsigned int width,
    unsigned int height,
    unsigned int stride,
    std::vector<unsigned char>& jpegBytes,
    std::string& error
) {
    jpegBytes.clear();

    ComPtr<IWICImagingFactory> factory;

    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to create WIC imaging factory.");
        return false;
    }

    ComPtr<IWICBitmap> bitmap;

    hr = factory->CreateBitmapFromMemory(
        width,
        height,
        GUID_WICPixelFormat32bppBGR,
        stride,
        stride * height,
        const_cast<unsigned char*>(pixels),
        &bitmap
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to create WIC bitmap from webcam frame.");
        return false;
    }

    ComPtr<IWICFormatConverter> converter;

    hr = factory->CreateFormatConverter(&converter);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to create WIC format converter.");
        return false;
    }

    hr = converter->Initialize(
        bitmap.Get(),
        GUID_WICPixelFormat24bppBGR,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to convert webcam frame to JPEG-compatible pixel format.");
        return false;
    }

    ComPtr<IStream> stream;

    hr = CreateStreamOnHGlobal(nullptr, TRUE, &stream);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to create memory stream for JPEG.");
        return false;
    }

    ComPtr<IWICBitmapEncoder> encoder;

    hr = factory->CreateEncoder(
        GUID_ContainerFormatJpeg,
        nullptr,
        &encoder
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to create JPEG encoder.");
        return false;
    }

    hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to initialize JPEG encoder.");
        return false;
    }

    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> propertyBag;

    hr = encoder->CreateNewFrame(&frame, &propertyBag);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to create JPEG frame.");
        return false;
    }

    if (propertyBag) {
        PROPBAG2 option{};
        option.pstrName = const_cast<LPOLESTR>(L"ImageQuality");

        VARIANT value{};
        VariantInit(&value);
        value.vt = VT_R4;
        value.fltVal = 0.92f;

        propertyBag->Write(1, &option, &value);
        VariantClear(&value);
    }

    hr = frame->Initialize(propertyBag.Get());

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to initialize JPEG frame.");
        return false;
    }

    hr = frame->SetSize(width, height);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to set JPEG frame size.");
        return false;
    }

    WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat24bppBGR;

    hr = frame->SetPixelFormat(&pixelFormat);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to set JPEG pixel format.");
        return false;
    }

    hr = frame->WriteSource(converter.Get(), nullptr);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to write JPEG pixels.");
        return false;
    }

    hr = frame->Commit();

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to commit JPEG frame.");
        return false;
    }

    hr = encoder->Commit();

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to commit JPEG encoder.");
        return false;
    }

    HGLOBAL memory = nullptr;

    hr = GetHGlobalFromStream(stream.Get(), &memory);

    if (FAILED(hr) || !memory) {
        error = HResultToString(hr, "Unable to get JPEG memory buffer.");
        return false;
    }

    SIZE_T size = GlobalSize(memory);

    if (size == 0) {
        error = "JPEG memory buffer is empty.";
        return false;
    }

    void* data = GlobalLock(memory);

    if (!data) {
        error = "Unable to lock JPEG memory buffer.";
        return false;
    }

    jpegBytes.assign(
        static_cast<unsigned char*>(data),
        static_cast<unsigned char*>(data) + size
    );

    GlobalUnlock(memory);

    return true;
}

bool WebcamCapture::CaptureJpeg(
    std::vector<unsigned char>& jpegBytes,
    std::string& error,
    int cameraIndex
) {
    jpegBytes.clear();
    error.clear();

    HRESULT comHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    bool shouldUninitializeCom = false;

    if (SUCCEEDED(comHr)) {
        shouldUninitializeCom = true;
    }
    else if (comHr != RPC_E_CHANGED_MODE) {
        error = HResultToString(comHr, "Unable to initialize COM.");
        return false;
    }

    HRESULT hr = MFStartup(MF_VERSION);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to start Media Foundation.");

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    ComPtr<IMFAttributes> deviceAttributes;

    hr = MFCreateAttributes(&deviceAttributes, 1);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to create Media Foundation attributes.");
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    hr = deviceAttributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to configure webcam device enumeration.");
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    IMFActivate** devices = nullptr;
    UINT32 deviceCount = 0;

    hr = MFEnumDeviceSources(
        deviceAttributes.Get(),
        &devices,
        &deviceCount
    );

    if (FAILED(hr) || deviceCount == 0) {
        error = "No webcam devices found.";
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    if (cameraIndex < 0 || static_cast<UINT32>(cameraIndex) >= deviceCount) {
        error = "Requested webcam index is not available.";

        for (UINT32 i = 0; i < deviceCount; ++i) {
            devices[i]->Release();
        }

        CoTaskMemFree(devices);
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    ComPtr<IMFMediaSource> mediaSource;

    hr = devices[cameraIndex]->ActivateObject(
        IID_PPV_ARGS(&mediaSource)
    );

    for (UINT32 i = 0; i < deviceCount; ++i) {
        devices[i]->Release();
    }

    CoTaskMemFree(devices);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to activate webcam.");
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    ComPtr<IMFAttributes> readerAttributes;

    hr = MFCreateAttributes(&readerAttributes, 1);

    if (SUCCEEDED(hr)) {
        readerAttributes->SetUINT32(
            MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING,
            TRUE
        );
    }

    ComPtr<IMFSourceReader> reader;

    hr = MFCreateSourceReaderFromMediaSource(
        mediaSource.Get(),
        readerAttributes.Get(),
        &reader
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to create webcam source reader.");
        mediaSource->Shutdown();
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    ComPtr<IMFMediaType> mediaType;

    hr = MFCreateMediaType(&mediaType);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to create webcam media type.");
        mediaSource->Shutdown();
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);

    hr = reader->SetCurrentMediaType(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        nullptr,
        mediaType.Get()
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to request RGB32 webcam output.");
        mediaSource->Shutdown();
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    ComPtr<IMFMediaType> currentType;

    hr = reader->GetCurrentMediaType(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        &currentType
    );

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to read webcam media type.");
        mediaSource->Shutdown();
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    UINT32 width = 0;
    UINT32 height = 0;

    hr = MFGetAttributeSize(
        currentType.Get(),
        MF_MT_FRAME_SIZE,
        &width,
        &height
    );

    if (FAILED(hr) || width == 0 || height == 0) {
        error = "Unable to determine webcam frame size.";
        mediaSource->Shutdown();
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    ComPtr<IMFSample> finalSample;

    for (int attempt = 0; attempt < 40; ++attempt) {
        DWORD streamIndex = 0;
        DWORD flags = 0;
        LONGLONG timestamp = 0;
        ComPtr<IMFSample> sample;

        hr = reader->ReadSample(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,
            &streamIndex,
            &flags,
            &timestamp,
            &sample
        );

        if (FAILED(hr)) {
            error = HResultToString(hr, "Unable to read webcam sample.");
            mediaSource->Shutdown();
            MFShutdown();

            if (shouldUninitializeCom) {
                CoUninitialize();
            }

            return false;
        }

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            break;
        }

        if (sample) {
            finalSample = sample;
        }

        if (attempt >= 5 && finalSample) {
            break;
        }

        Sleep(50);
    }

    if (!finalSample) {
        error = "No webcam frame was captured.";
        mediaSource->Shutdown();
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    ComPtr<IMFMediaBuffer> buffer;

    hr = finalSample->ConvertToContiguousBuffer(&buffer);

    if (FAILED(hr)) {
        error = HResultToString(hr, "Unable to get webcam frame buffer.");
        mediaSource->Shutdown();
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    BYTE* rawData = nullptr;
    DWORD maxLength = 0;
    DWORD currentLength = 0;

    hr = buffer->Lock(&rawData, &maxLength, &currentLength);

    if (FAILED(hr) || !rawData) {
        error = HResultToString(hr, "Unable to lock webcam frame buffer.");
        mediaSource->Shutdown();
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    const unsigned int stride = width * 4;
    const unsigned int expectedSize = stride * height;

    if (currentLength < expectedSize) {
        buffer->Unlock();

        error = "Webcam frame buffer is smaller than expected.";
        mediaSource->Shutdown();
        MFShutdown();

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        return false;
    }

    bool ok = EncodeRgb32ToJpeg(
        rawData,
        width,
        height,
        stride,
        jpegBytes,
        error
    );

    buffer->Unlock();

    mediaSource->Shutdown();
    MFShutdown();

    if (shouldUninitializeCom) {
        CoUninitialize();
    }

    return ok;
}
