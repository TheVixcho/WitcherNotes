#include "UpdateManager.h"
#include "SettingsManager.h"
#include "AppPaths.h"
#include "Version.h"
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#endif

using json = nlohmann::json;
using namespace WitcherNotes::Settings;

namespace WitcherNotes::Core {

// ─────────────────────────────────────────────────────────────────────────────
// SemVer parser y comparador (compatible con pre-releases y etiquetas)
// ─────────────────────────────────────────────────────────────────────────────

struct SemVer {
    int major = 0, minor = 0, patch = 0;
    std::vector<std::string> pre;
};

static SemVer ParseSemVer(const std::string& v) {
    SemVer sv;
    std::string clean = v;
    if (!clean.empty() && (clean[0] == 'v' || clean[0] == 'V')) {
        clean = clean.substr(1);
    }

    std::string core = clean, pre;
    auto dash = clean.find('-');
    if (dash != std::string::npos) {
        core = clean.substr(0, dash);
        pre = clean.substr(dash + 1);
    }
    sscanf(core.c_str(), "%d.%d.%d", &sv.major, &sv.minor, &sv.patch);

    size_t start = 0;
    while (!pre.empty() && start <= pre.size()) {
        size_t dot = pre.find('.', start);
        sv.pre.push_back(pre.substr(start, dot == std::string::npos ? std::string::npos : dot - start));
        if (dot == std::string::npos) break;
        start = dot + 1;
    }
    return sv;
}

static bool IsAllDigits(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); });
}

static bool ComparePrerelease(const std::vector<std::string>& a, const std::vector<std::string>& b) {
    size_t n = std::min(a.size(), b.size());
    for (size_t i = 0; i < n; i++) {
        if (a[i] == b[i]) continue;
        bool na = IsAllDigits(a[i]), nb = IsAllDigits(b[i]);
        if (na && nb) return std::stoll(a[i]) < std::stoll(b[i]);
        if (na != nb) return na;
        return a[i] < b[i];
    }
    return a.size() < b.size();
}

bool UpdateManager::IsNewerVersion(const std::string& currentVersion, const std::string& remoteVersion) {
    SemVer c = ParseSemVer(currentVersion);
    SemVer l = ParseSemVer(remoteVersion);

    if (l.major != c.major) return l.major > c.major;
    if (l.minor != c.minor) return l.minor > c.minor;
    if (l.patch != c.patch) return l.patch > c.patch;

    if (c.pre.empty()) return false;
    if (l.pre.empty()) return true;
    return ComparePrerelease(c.pre, l.pre);
}

// ─────────────────────────────────────────────────────────────────────────────
// Ejecutor de comandos HTTP via curl con soporte para container/host
// ─────────────────────────────────────────────────────────────────────────────

static std::string ExecuteCurl(const std::string& url, const std::string& token, const std::string& acceptHeader = "application/vnd.github+json") {
    std::string authHeader = "";
    if (!token.empty()) {
        authHeader = "-H \"Authorization: Bearer " + token + "\" ";
    }

    std::string cmd = "curl -s -L -H \"User-Agent: CuadernoDeNotas-Updater\" "
                      "-H \"Accept: " + acceptHeader + "\" "
                      "-H \"X-GitHub-Api-Version: 2022-11-28\" "
                      + authHeader + "\"" + url + "\"";

    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        // Fallback a flatpak-spawn si estamos dentro del sandbox
        cmd = "flatpak-spawn --host " + cmd;
        pipe = popen(cmd.c_str(), "r");
    }

    if (pipe) {
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
    }

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Implementación de UpdateManager
// ─────────────────────────────────────────────────────────────────────────────

UpdateManager::UpdateManager() = default;

UpdateManager::~UpdateManager() {
    if (m_WorkerThread.joinable()) {
        m_WorkerThread.join();
    }
}

UpdateManager& UpdateManager::Get() {
    static UpdateManager s_Instance;
    return s_Instance;
}

void UpdateManager::CheckOnStartup() {
    auto& s = SettingsManager::Get().GetSettings();
    if (s.checkUpdatesOnStartup) {
        CheckForUpdates(false);
    }
}

void UpdateManager::CheckForUpdates(bool showModalIfAvailable) {
    if (m_Status.load() == UpdateStatus::Checking || m_IsDownloading.load()) {
        return;
    }

    if (m_WorkerThread.joinable()) {
        m_WorkerThread.join();
    }

    m_Status = UpdateStatus::Checking;
    m_ErrorMsg.clear();

    m_WorkerThread = std::thread(&UpdateManager::WorkerCheck, this, showModalIfAvailable);
}

void UpdateManager::WorkerCheck(bool showModalIfAvailable) {
    auto& s = SettingsManager::Get().GetSettings();
    std::string repo = s.githubRepo.empty() ? "TheVixcho/CuadernoDeNotas" : s.githubRepo;
    std::string token = s.githubToken;
    std::string channel = s.updateChannel;

    std::string apiUrl = "https://api.github.com/repos/" + repo + "/releases";
    if (channel == "beta") {
        apiUrl += "?per_page=1";
    } else {
        apiUrl += "/latest";
    }

    std::string response = ExecuteCurl(apiUrl, token);
    if (response.empty()) {
        m_ErrorMsg = "No se pudo conectar con el servidor de GitHub.";
        m_Status = UpdateStatus::Error;
        return;
    }

    try {
        json j = json::parse(response);
        if (j.is_array() && !j.empty()) {
            j = j[0];
        }

        if (j.contains("message")) {
            std::string msg = j["message"].get<std::string>();
            if (msg.find("Not Found") != std::string::npos) {
                m_ErrorMsg = "Repositorio no encontrado. Si es privado, verifica tu token de GitHub.";
            } else if (msg.find("Bad credentials") != std::string::npos) {
                m_ErrorMsg = "Token de GitHub no válido o expirado.";
            } else {
                m_ErrorMsg = "GitHub API: " + msg;
            }
            m_Status = UpdateStatus::Error;
            return;
        }

        m_ReleaseInfo.tagName = j.value("tag_name", "");
        m_ReleaseInfo.version = m_ReleaseInfo.tagName;
        if (!m_ReleaseInfo.version.empty() && (m_ReleaseInfo.version[0] == 'v' || m_ReleaseInfo.version[0] == 'V')) {
            m_ReleaseInfo.version = m_ReleaseInfo.version.substr(1);
        }

        m_ReleaseInfo.title = j.value("name", "");
        m_ReleaseInfo.changelog = j.value("body", "");
        m_ReleaseInfo.publishedAt = j.value("published_at", "");
        m_LatestVersion = m_ReleaseInfo.version;
        m_Changelog = m_ReleaseInfo.changelog;

        // Buscar el asset correspondiente al sistema operativo
        m_ReleaseInfo.asset = ReleaseAssetInfo{};
        if (j.contains("assets") && j["assets"].is_array()) {
            for (const auto& a : j["assets"]) {
                std::string name = a.value("name", "");
                std::string nameLower = name;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

#if defined(_WIN32)
                if (nameLower.find(".exe") != std::string::npos) {
                    m_ReleaseInfo.asset.id = a.value("id", 0);
                    m_ReleaseInfo.asset.name = name;
                    m_ReleaseInfo.asset.size = a.value("size", (size_t)0);
                    m_ReleaseInfo.asset.apiUrl = a.value("url", "");
                    m_ReleaseInfo.asset.browserDownloadUrl = a.value("browser_download_url", "");
                    break;
                }
#else
                if (nameLower.find(".appimage") != std::string::npos) {
                    m_ReleaseInfo.asset.id = a.value("id", 0);
                    m_ReleaseInfo.asset.name = name;
                    m_ReleaseInfo.asset.size = a.value("size", (size_t)0);
                    m_ReleaseInfo.asset.apiUrl = a.value("url", "");
                    m_ReleaseInfo.asset.browserDownloadUrl = a.value("browser_download_url", "");
                    break;
                }
#endif
            }
        }

        // Registrar timestamp de última comprobación
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char timeBuf[32];
        strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", localtime(&now));
        s.lastUpdateCheck = timeBuf;
        SettingsManager::Get().Save();

        // Comparar SemVer con APP_VERSION_STRING
        if (IsNewerVersion(APP_VERSION_STRING, m_ReleaseInfo.version)) {
            m_Status = UpdateStatus::Available;
            if (showModalIfAvailable) {
                m_ShowModal = true;
            }
        } else {
            m_Status = UpdateStatus::UpToDate;
        }

    } catch (const std::exception& e) {
        m_ErrorMsg = std::string("Error al procesar respuesta: ") + e.what();
        m_Status = UpdateStatus::Error;
    }
}

void UpdateManager::DownloadAndInstall() {
    if (m_Status.load() != UpdateStatus::Available || m_IsDownloading.load()) {
        return;
    }

    if (m_WorkerThread.joinable()) {
        m_WorkerThread.join();
    }

    m_IsDownloading = true;
    m_DownloadProgress = 0.0f;
    m_DownloadSpeedMBs = 0.0f;
    m_DownloadedMB = 0.0f;
    m_TotalMB = (float)m_ReleaseInfo.asset.size / (1024.0f * 1024.0f);

    m_WorkerThread = std::thread(&UpdateManager::WorkerDownload, this);
}

void UpdateManager::WorkerDownload() {
    auto& s = SettingsManager::Get().GetSettings();
    std::string token = s.githubToken;
    const auto& asset = m_ReleaseInfo.asset;

    // Determinar destino del archivo
    std::string destDir = AppPaths::GetAppDataDir();
    std::string destFile;

#if defined(_WIN32)
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    destFile = std::string(tempPath) + (asset.name.empty() ? "CuadernoDeNotas_Setup.exe" : asset.name);
#else
    const char* appImageEnv = std::getenv("APPIMAGE");
    std::string currentAppImage = appImageEnv ? appImageEnv : "";

    if (!currentAppImage.empty()) {
        destFile = currentAppImage + ".new";
    } else {
        destFile = destDir + "/" + (asset.name.empty() ? "CuadernoDeNotas-x86_64.AppImage" : asset.name);
    }
#endif

    m_DownloadedFilePath = destFile;
    std::error_code ecRem;
    std::filesystem::remove(destFile, ecRem);

    // Preparar comando curl para descarga
    // Para repos privados: descargar via API asset endpoint con 'Accept: application/octet-stream'
    // Para repos públicos: descargar via browser_download_url
    std::string downloadUrl;
    std::string headers;

    if (!token.empty() && !asset.apiUrl.empty()) {
        downloadUrl = asset.apiUrl;
        headers = "-H \"Authorization: Bearer " + token + "\" "
                  "-H \"Accept: application/octet-stream\" "
                  "-H \"X-GitHub-Api-Version: 2022-11-28\" ";
    } else {
        downloadUrl = asset.browserDownloadUrl;
        headers = "-H \"Accept: application/octet-stream\" ";
    }

    std::string dlCmd = "curl -s -L -H \"User-Agent: CuadernoDeNotas-Updater\" "
                        + headers + "-o \"" + destFile + "\" \"" + downloadUrl + "\"";

    std::atomic<bool> dlDone{ false };
    int retCode = -1;

    std::thread dlThread([dlCmd, &dlDone, &retCode]() {
        retCode = system(dlCmd.c_str());
        dlDone = true;
    });

    float totalBytes = (float)asset.size;
    auto lastTime = std::chrono::steady_clock::now();
    float lastBytes = 0.0f;

    while (!dlDone.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (std::filesystem::exists(destFile)) {
            std::error_code ec;
            float currentSize = (float)std::filesystem::file_size(destFile, ec);
            if (!ec) {
                m_DownloadedMB = currentSize / (1024.0f * 1024.0f);
                if (totalBytes > 0.0f) {
                    m_DownloadProgress = std::min(0.99f, currentSize / totalBytes);
                }

                auto now = std::chrono::steady_clock::now();
                float sec = std::chrono::duration<float>(now - lastTime).count();
                if (sec >= 0.35f) {
                    float diff = currentSize - lastBytes;
                    m_DownloadSpeedMBs = (diff / (1024.0f * 1024.0f)) / sec;
                    lastBytes = currentSize;
                    lastTime = now;
                }
            }
        }
    }

    if (dlThread.joinable()) {
        dlThread.join();
    }

    m_IsDownloading = false;

    if (retCode == 0 && std::filesystem::exists(destFile)) {
        m_DownloadProgress = 1.0f;

#if defined(_WIN32)
        ShellExecuteA(NULL, "open", destFile.c_str(), NULL, NULL, SW_SHOWNORMAL);
        exit(0);
#else
        // Establecer permisos de ejecución en Linux
        std::error_code ecPerm;
        std::filesystem::permissions(destFile,
            std::filesystem::perms::owner_all |
            std::filesystem::perms::group_read | std::filesystem::perms::group_exec |
            std::filesystem::perms::others_read | std::filesystem::perms::others_exec,
            std::filesystem::perm_options::replace, ecPerm);

        const char* aiEnv = std::getenv("APPIMAGE");
        if (aiEnv && std::string(aiEnv) != "") {
            std::string curAi = aiEnv;
            // Script de reemplazo y reinicio automático
            std::string script = "/tmp/apply_update_" + std::to_string(getpid()) + ".sh";
            {
                std::ofstream sc(script);
                sc << "#!/bin/sh\n";
                sc << "sleep 1\n";
                sc << "mv -f \"" << destFile << "\" \"" << curAi << "\"\n";
                sc << "chmod +x \"" << curAi << "\"\n";
                sc << "exec \"" << curAi << "\" &\n";
                sc << "rm -f \"$0\"\n";
            }
            std::system(("chmod +x \"" + script + "\"").c_str());
            std::system(("sh \"" + script + "\" &").c_str());
            exit(0);
        }
#endif
    } else {
        m_ErrorMsg = "Error durante la descarga del archivo de actualización.";
        m_Status = UpdateStatus::Error;
    }
}

} // namespace WitcherNotes::Core
