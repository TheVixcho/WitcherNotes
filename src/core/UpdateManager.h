#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <vector>

namespace WitcherNotes::Core {

enum class UpdateStatus {
    Idle = 0,
    Checking,
    UpToDate,
    Available,
    Error
};

struct ReleaseAssetInfo {
    int id = 0;
    std::string name;
    size_t size = 0;
    std::string apiUrl;
    std::string browserDownloadUrl;
};

struct UpdateReleaseInfo {
    std::string tagName;
    std::string version;
    std::string title;
    std::string changelog;
    std::string publishedAt;
    ReleaseAssetInfo asset;
};

class UpdateManager {
public:
    static UpdateManager& Get();

    void CheckOnStartup();
    void CheckForUpdates(bool showModalIfAvailable = true);
    void DownloadAndInstall();

    // Estado del updater
    UpdateStatus GetStatus() const { return m_Status.load(); }
    const std::string& GetLatestVersion() const { return m_LatestVersion; }
    const std::string& GetChangelog() const { return m_Changelog; }
    const std::string& GetErrorMessage() const { return m_ErrorMsg; }

    // Progreso de descarga
    bool IsDownloading() const { return m_IsDownloading.load(); }
    float GetDownloadProgress() const { return m_DownloadProgress.load(); }
    float GetDownloadSpeedMBs() const { return m_DownloadSpeedMBs.load(); }
    float GetDownloadedMB() const { return m_DownloadedMB.load(); }
    float GetTotalMB() const { return m_TotalMB.load(); }

    // Modal global
    bool ShouldShowModal() const { return m_ShowModal; }
    void SetShowModal(bool show) { m_ShowModal = show; }

    // Helpers
    static bool IsNewerVersion(const std::string& currentVersion, const std::string& remoteVersion);

private:
    UpdateManager();
    ~UpdateManager();

    void WorkerCheck(bool showModalIfAvailable);
    void WorkerDownload();

    std::atomic<UpdateStatus> m_Status{ UpdateStatus::Idle };
    std::string m_LatestVersion;
    std::string m_Changelog;
    std::string m_ErrorMsg;
    UpdateReleaseInfo m_ReleaseInfo;

    std::atomic<bool> m_IsDownloading{ false };
    std::atomic<float> m_DownloadProgress{ 0.0f };
    std::atomic<float> m_DownloadSpeedMBs{ 0.0f };
    std::atomic<float> m_DownloadedMB{ 0.0f };
    std::atomic<float> m_TotalMB{ 0.0f };

    bool m_ShowModal = false;
    std::string m_DownloadedFilePath;

    std::thread m_WorkerThread;
};

} // namespace WitcherNotes::Core

