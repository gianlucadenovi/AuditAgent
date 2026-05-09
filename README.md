<p align="center">
  <img src="auditagent.png" alt="Audit Agent logo" width="180">
</p>

<h1 align="center">Audit Agent</h1>

<p align="center">
  Lightweight Windows diagnostics webserver for authorized local testing, KVM workflows, and controlled host inspection.
</p>


<p align="center">
  <img alt="OS" src="https://img.shields.io/badge/OS-Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white">
  <img alt="Language" src="https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white">
  <img alt="Build" src="https://img.shields.io/badge/Build-CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white">
  <img alt="Webserver" src="https://img.shields.io/badge/Webserver-cpp--httplib-FF6F00?style=for-the-badge">
  <img alt="JSON" src="https://img.shields.io/badge/JSON-JsonCpp-8A2BE2?style=for-the-badge">
  <img alt="Database" src="https://img.shields.io/badge/Database-SQLite-003B57?style=for-the-badge&logo=sqlite&logoColor=white">
  <img alt="Windows APIs" src="https://img.shields.io/badge/APIs-Media%20Foundation%20%7C%20WASAPI%20%7C%20GDI-00A4EF?style=for-the-badge">
  <img alt="License" src="https://img.shields.io/badge/License-Apache%202.0-lightgrey?style=for-the-badge">
</p>


It provides a browser-based dashboard for capturing host screenshots, webcam frames, short audio clips, file previews/downloads, normal browser history, selected system file inspection, and basic host control actions.

> **Audit Agent is intended only for systems you own or are explicitly authorized to test.**

---

## Features

### Secure access

- Local web dashboard available at:

```text
http://127.0.0.1:8787/login
```

- API key is required to open the dashboard.
- On first launch, Audit Agent stores only the SHA-256 hash of the API key in:

```text
system_ak.txc
```

- Future API requests send the API key inside the JSON body.

---

### Dashboard pages

#### Desktop

- Capture host desktop screenshots.
- Auto-refresh screenshots every N seconds.
- Save screenshots from the browser.

#### Files

- Browse the configured allowed root folder.
- Preview text, image, and video files.
- Download selected files as a ZIP archive.

#### Browser History

- Read normal browser history from supported browsers:
  - Chrome
  - Edge
  - Firefox
- Private/incognito history is not available.

#### Webcam

- Capture JPEG frames from the host webcam.
- Select camera index.
- Auto-capture frames every N seconds.
- Save webcam frames.

#### Audio

- Capture short WAV clips from the host microphone.
- Select capture duration.
- Auto-capture audio clips.

#### System Sec

- Read `system_sec.txc` from the Audit Agent executable folder.
- Decode its content by inverting each byte bit-by-bit.
- Reset/recreate the file from the dashboard.

#### Host Control

- Force reboot the host machine.
- Requires confirmation prompts in the dashboard.

---
####Architecture

This software is meant to be used in local networks, connecting directly to the host.
```
Remote PC running AuditAgent
        ^
        |
        V
Your browser connects here
```

If you wish to reach the host from outside you would have to implement an outbound connection setting up a proper triangulation server.

```
Remote PC running AuditAgent
        |
        | outbound connection
        v
Your public VPS / relay server
        ^
        |
Your browser connects here
```
---

## Requirements

- Windows 10 or Windows 11
- Visual Studio 2022 or compatible MSVC toolchain
- C++17
- Windows SDK
- JsonCpp
- SQLite source included directly as `sqlite3.c`

No OpenCV is required.

---

## Project structure

```text
AuditAgent/
  main.cpp
  AuditWebServer.h
  AuditWebServer.cpp
  AuditAgentUI.h
  AuditAgentUI.cpp

  ApiKeyManager.h
  ApiKeyManager.cpp

  ScreenCapture.h
  ScreenCapture.cpp

  WebcamCapture.h
  WebcamCapture.cpp

  AudioCapture.h
  AudioCapture.cpp

  FileBrowser.h
  FileBrowser.cpp

  BrowserHistory.h
  BrowserHistory.cpp

  ZipBuilder.h
  ZipBuilder.cpp

  HostControl.h
  HostControl.cpp

  SystemSecFile.h
  SystemSecFile.cpp

  sqlite3.h
  sqlite3.c

  httplib.h
```


## Build

### Requirements

- Windows 10 / 11
- Visual Studio 2022 or compatible MSVC toolchain
- CMake 3.20+
- C++17
- Windows SDK
- JsonCpp
- cpp-httplib
- SQLite source included directly as `sqlite3.c`

### Build with CMake

From the project root:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

---

## First launch

On first launch, provide an API key:

```powershell
.\AuditAgent.exe k=YourStrongPasswordHere
```

Audit Agent will create:

```text
system_ak.txc
```

This file contains the SHA-256 hash of the API key.

After that, future launches do not require the key argument:

```powershell
.\AuditAgent.exe
```

---

## Open the dashboard

After starting the program, open:

```text
http://127.0.0.1:8787/login
```

Enter the API key used during first launch.

---

## Build command

Example MSVC command:

```bat
cl /EHsc /std:c++17 main.cpp AuditWebServer.cpp AuditAgentUI.cpp ScreenCapture.cpp FileBrowser.cpp ZipBuilder.cpp BrowserHistory.cpp WebcamCapture.cpp AudioCapture.cpp HostControl.cpp SystemSecFile.cpp ApiKeyManager.cpp sqlite3.c /Fe:AuditAgent.exe user32.lib gdi32.lib mfplat.lib mf.lib mfreadwrite.lib mfuuid.lib windowscodecs.lib ole32.lib uuid.lib bcrypt.lib jsoncpp.lib
```

Depending on your JsonCpp setup, you may need to adjust include/library paths.

---

## Visual Studio setup

Add all `.cpp` and `.h` files to your Visual Studio project.

Required linker inputs:

```text
user32.lib
gdi32.lib
mfplat.lib
mf.lib
mfreadwrite.lib
mfuuid.lib
windowscodecs.lib
ole32.lib
uuid.lib
bcrypt.lib
jsoncpp.lib
```

Use:

```text
C++ Language Standard: C++17
```

If `sqlite3.c` gives C++ warnings or errors, set that file to compile as C code.

---

## Installation script example

Audit Agent can be installed with a PowerShell one-liner.

Example:

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -Command "& ([scriptblock]::Create((irm 'https://esacode.ai/install/AuditAgent.ps1'))) -ApiKey 'YourStrongPasswordHere'"
```

The installer should:

1. Download `AuditAgent.zip`.
2. Extract it to:

```text
C:\ProgramData\AuditAgent
```

3. Start Audit Agent once with:

```text
k=YourStrongPasswordHere
```

4. Create a startup `.bat` file in the Windows Startup folder.
5. Launch Audit Agent automatically on future logins.

---

## Example installer script

```powershell
param(
    [Parameter(Mandatory = $true)]
    [string]$ApiKey,

    [string]$PackageUrl = "https://esacode.ai/install/AuditAgent.zip"
)

$ErrorActionPreference = "Stop"

$InstallDir = "$env:ProgramData\AuditAgent"
$ZipPath = "$env:TEMP\AuditAgent.zip"
$ExePath = Join-Path $InstallDir "AuditAgent.exe"
$StartupBat = "$env:APPDATA\Microsoft\Windows\Start Menu\Programs\Startup\AuditAgent.bat"
$KeyFile = Join-Path $InstallDir "system_ak.txc"

Write-Host "Installing AuditAgent..."

New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null

Write-Host "Downloading package..."
Invoke-WebRequest -Uri $PackageUrl -OutFile $ZipPath

Write-Host "Extracting package..."
Expand-Archive -Path $ZipPath -DestinationPath $InstallDir -Force

Remove-Item $ZipPath -Force -ErrorAction SilentlyContinue

if (!(Test-Path $ExePath)) {
    Write-Host "ERROR: AuditAgent.exe not found in package."
    exit 1
}

Write-Host "Creating startup launcher..."

$BatContent = @"
@echo off
cd /d "$InstallDir"
start "" "$ExePath"
"@

Set-Content -Path $StartupBat -Value $BatContent -Encoding ASCII

Write-Host "Starting AuditAgent..."

if (!(Test-Path $KeyFile)) {
    Start-Process -FilePath $ExePath -ArgumentList "k=$ApiKey" -WorkingDirectory $InstallDir
} else {
    Start-Process -FilePath $ExePath -WorkingDirectory $InstallDir
}

Write-Host ""
Write-Host "AuditAgent installed."
Write-Host "Login URL:"
Write-Host "http://127.0.0.1:8787/login"
```

---

## Security notes

Audit Agent is designed for controlled, authorized diagnostics.

Recommended usage:

- Run only on machines you own or are authorized to inspect.
- Keep the API key private.
- Use HTTPS before exposing beyond localhost.
- Keep the webserver bound to `127.0.0.1` unless you explicitly need network access.
- Do not deploy on third-party systems without consent.

The current default host binding is:

```text
127.0.0.1
```

---

## API model

Protected API routes use POST requests with JSON bodies.

Example request body:

```json
{
  "api_key": "YourStrongPasswordHere"
}
```

Example file list request:

```json
{
  "api_key": "YourStrongPasswordHere",
  "path": "C:\\"
}
```

---

## Main routes

```text
GET  /                  -> 404 Not Found
GET  /login             -> Login page
POST /ui                -> Dashboard HTML

POST /api/health
POST /api/screenshot
POST /api/files/list
POST /api/files/download
POST /api/files/download-zip
POST /api/browser/available
POST /api/browser/history
POST /api/webcam/frame
POST /api/audio/capture
POST /api/system-sec/read
POST /api/system-sec/reset
POST /api/host/reboot
```
---
## Disclaimer

Audit Agent is provided for lawful, authorized, and ethical use only.

The software is intended for diagnostics, testing, system administration, KVM workflows, and controlled inspection of systems that you own or are explicitly authorized to access.

By using this software, you agree that:

- You are solely responsible for how you use, modify, deploy, or distribute this code.
- You will not use this software to access, monitor, inspect, capture, or control any system without proper authorization.
- The authors, contributors, and copyright holders are not responsible for any misuse, damage, data loss, privacy violation, legal consequence, or unauthorized activity resulting from the use of this software.
- This software is provided “as is,” without warranty of any kind.

If you do not have explicit permission to use Audit Agent on a system, do not install or run it.
---

## License

Apache License
Version 2.0, January 2004
http://www.apache.org/licenses/


Example:

```text
Copyright (c) Esacode.
All rights reserved.
```


```
