#include "SettingsPanel.h"
#include "core/SettingsManager.h"
#include "core/BookModel.h"
#include "core/AppPaths.h"
#include "core/UpdateManager.h"
#include "Version.h"
#include <imgui.h>
#include <cstring>
#include <algorithm>

namespace WitcherNotes::UI {

using namespace WitcherNotes::Core;
using namespace WitcherNotes::Settings;

static ImVec4 ToVec4(const float c[4], float alpha = -1.0f) {
    float a = (alpha >= 0.0f) ? alpha : c[3];
    return ImVec4(c[0], c[1], c[2], a);
}

SettingsPanel::SettingsPanel() {}

void SettingsPanel::Render() {
    auto& sm = SettingsManager::Get();
    auto& s = sm.GetSettings();
    ThemeColors theme = sm.GetCurrentThemeColors();

    ImGui::TextColored(ToVec4(theme.accentLight), "Settings & Preferences");
    ImGui::Separator();
    ImGui::Spacing();

    // Section 1: Themes & Appearance
    if (ImGui::CollapsingHeader("Theme & Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ToVec4(theme.textDim), "Select the interface color scheme:");
        ImGui::Spacing();

        const ThemePreset presets[] = {
            ThemePreset::DarkModern,
            ThemePreset::Paper,
            ThemePreset::DeepPurple,
            ThemePreset::Sapphire,
            ThemePreset::MinimalLight,
            ThemePreset::SepiaWarm
        };

        for (int i = 0; i < 6; ++i) {
            bool isCurrent = (s.theme == presets[i]);
            if (ImGui::RadioButton(SettingsManager::GetThemeName(presets[i]), isCurrent)) {
                s.theme = presets[i];
                sm.ApplyTheme();
                sm.Save();
            }
        }
    }

    ImGui::Spacing();

    // Section 2: Reader Settings
    if (ImGui::CollapsingHeader("Reader Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* modeNames[] = {
            "Two Pages (Book spread)",
            "Single Page (Centered)",
            "Continuous Scroll"
        };
        int curMode = static_cast<int>(s.readerMode);
        ImGui::Text("Viewing Mode:");
        if (ImGui::Combo("##ReaderMode", &curMode, modeNames, 3)) {
            s.readerMode = static_cast<ReaderMode>(curMode);
            sm.Save();
        }

        ImGui::Spacing();

        ImGui::Text("Reader Font Size:");
        if (ImGui::SliderFloat("##FontSize", &s.readerFontSize, 12.0f, 28.0f, "%.0f pt")) {
            sm.Save();
        }

        ImGui::Text("Page Margins:");
        if (ImGui::SliderFloat("##PageMargin", &s.readerPageMargin, 16.0f, 64.0f, "%.0f px")) {
            sm.Save();
        }

        ImGui::Spacing();

        if (ImGui::Checkbox("Page Borders & Frames", &s.showPageBorders)) {
            sm.Save();
        }

        if (ImGui::Checkbox("Center Spine Shadow", &s.showSpineShadow)) {
            sm.Save();
        }

        if (ImGui::Checkbox("Show Marginalia Notes", &s.showMarginalia)) {
            sm.Save();
        }
    }

    ImGui::Spacing();

    // Section 3: Updates
    if (ImGui::CollapsingHeader("Application Updates", ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderCategoryUpdates();
    }

    ImGui::Spacing();

    // Section 4: Data & Backups
    if (ImGui::CollapsingHeader("Backup & Storage")) {
        if (ImGui::Button("Save All Now (Ctrl+S)", ImVec2(220, 32))) {
            BookRepository::Get().Save();
            sm.Save();
        }

        ImGui::Spacing();

        if (ImGui::Button("Export Active Notebook to Markdown", ImVec2(260, 32))) {
            auto* b = BookRepository::Get().GetActiveBook();
            if (b) {
                std::string path = AppPaths::GetBackupsDir() + "/" + b->title + ".md";
                BookRepository::Get().ExportActiveBookToMarkdown(path);
            }
        }

        ImGui::Spacing();

        if (ImGui::Button("Restore Sample Notebooks", ImVec2(240, 32))) {
            BookRepository::Get().PopulateDefaultsIfEmpty();
        }

        ImGui::Spacing();
        ImGui::TextColored(ToVec4(theme.textFaint), "Data directory: %s", AppPaths::GetAppDataDir().c_str());
    }
}

void SettingsPanel::RenderCategoryUpdates() {
    auto& sm = SettingsManager::Get();
    auto& s = sm.GetSettings();
    auto& updater = UpdateManager::Get();
    ThemeColors theme = sm.GetCurrentThemeColors();

    if (!m_BuffersInitialized) {
        strncpy(m_RepoBuffer, s.githubRepo.c_str(), sizeof(m_RepoBuffer) - 1);
        strncpy(m_TokenBuffer, s.githubToken.c_str(), sizeof(m_TokenBuffer) - 1);
        m_BuffersInitialized = true;
    }

    // Installed version badge
    ImGui::TextColored(ToVec4(theme.textDim), "Installed version:");
    ImGui::SameLine();
    ImGui::TextColored(ToVec4(theme.accentLight), "v%s", APP_VERSION_STRING);

    if (!s.lastUpdateCheck.empty()) {
        ImGui::TextColored(ToVec4(theme.textFaint), "Last checked: %s", s.lastUpdateCheck.c_str());
    }

    ImGui::Spacing();

    // Update status card
    UpdateStatus st = updater.GetStatus();
    ImVec4 borderCol = ToVec4(theme.border);
    ImVec4 bgCol = ToVec4(theme.surface0);
    ImVec4 textCol = ToVec4(theme.textPrimary);
    std::string statusTitle = "";
    std::string statusDetail = "";

    switch (st) {
        case UpdateStatus::Idle:
            statusTitle = "Not checked";
            statusDetail = "Click 'Check Now' to search for new versions.";
            break;
        case UpdateStatus::Checking:
            borderCol = ImVec4(0.3f, 0.5f, 0.9f, 0.8f);
            bgCol = ImVec4(0.08f, 0.12f, 0.22f, 1.0f);
            textCol = ImVec4(0.6f, 0.8f, 1.0f, 1.0f);
            statusTitle = "Checking for updates...";
            statusDetail = "Connecting to GitHub API...";
            break;
        case UpdateStatus::UpToDate:
            borderCol = ImVec4(0.2f, 0.7f, 0.35f, 0.8f);
            bgCol = ImVec4(0.05f, 0.15f, 0.08f, 1.0f);
            textCol = ImVec4(0.4f, 0.9f, 0.5f, 1.0f);
            statusTitle = "Up to date";
            statusDetail = "You have the latest version installed.";
            break;
        case UpdateStatus::Available:
            borderCol = ImVec4(0.9f, 0.6f, 0.1f, 0.9f);
            bgCol = ImVec4(0.20f, 0.14f, 0.04f, 1.0f);
            textCol = ImVec4(1.0f, 0.8f, 0.3f, 1.0f);
            statusTitle = "New version available: v" + updater.GetLatestVersion();
            if (!updater.GetChangelog().empty()) {
                statusDetail = updater.GetChangelog();
            } else {
                statusDetail = "An update is available and ready to install.";
            }
            break;
        case UpdateStatus::Error:
            borderCol = ImVec4(0.85f, 0.25f, 0.25f, 0.8f);
            bgCol = ImVec4(0.18f, 0.05f, 0.05f, 1.0f);
            textCol = ImVec4(1.0f, 0.45f, 0.45f, 1.0f);
            statusTitle = "Error checking for updates";
            statusDetail = updater.GetErrorMessage();
            break;
    }

    // Draw status card
    ImVec2 boxPos = ImGui::GetCursorScreenPos();
    float boxW = ImGui::GetContentRegionAvail().x;
    float boxH = (st == UpdateStatus::Available && !updater.GetChangelog().empty()) ? 100.0f : 66.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(boxPos, ImVec2(boxPos.x + boxW, boxPos.y + boxH), ImGui::ColorConvertFloat4ToU32(bgCol), 6.0f);
    dl->AddRect(boxPos, ImVec2(boxPos.x + boxW, boxPos.y + boxH), ImGui::ColorConvertFloat4ToU32(borderCol), 6.0f, 0, 1.0f);

    ImGui::SetCursorScreenPos(ImVec2(boxPos.x + 14.0f, boxPos.y + 10.0f));
    ImGui::TextColored(textCol, "%s", statusTitle.c_str());

    if (!statusDetail.empty()) {
        ImGui::SetCursorScreenPos(ImVec2(boxPos.x + 14.0f, boxPos.y + 32.0f));
        ImGui::PushTextWrapPos(boxPos.x + boxW - 14.0f);
        ImGui::TextColored(ToVec4(theme.textDim), "%s", statusDetail.c_str());
        ImGui::PopTextWrapPos();
    }

    ImGui::SetCursorScreenPos(ImVec2(boxPos.x, boxPos.y + boxH + 10.0f));

    // Active download progress bar
    if (updater.IsDownloading()) {
        float pct = updater.GetDownloadProgress();
        float dlMB = updater.GetDownloadedMB();
        float totMB = updater.GetTotalMB();
        float speed = updater.GetDownloadSpeedMBs();

        ImGui::TextColored(ToVec4(theme.accentLight), "Downloading update...");
        ImGui::ProgressBar(pct, ImVec2(ImGui::GetContentRegionAvail().x, 14.0f), "");

        if (totMB > 0.0f) {
            ImGui::TextColored(ToVec4(theme.textDim), "%.1f MB / %.1f MB  (%.1f MB/s)  %d%%",
                               dlMB, totMB, speed, (int)(pct * 100.0f));
        } else {
            ImGui::TextColored(ToVec4(theme.textDim), "Downloading... (%.1f MB/s)", speed);
        }
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Do not close the application during download.");
        ImGui::Spacing();
    }

    // Action buttons
    bool isBusy = (st == UpdateStatus::Checking || updater.IsDownloading());
    if (isBusy) ImGui::BeginDisabled();

    if (ImGui::Button("Check Now", ImVec2(160.0f, 30.0f))) {
        updater.CheckForUpdates(true);
    }

    if (st == UpdateStatus::Available && !updater.IsDownloading()) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ToVec4(theme.accent));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.08f, 0.05f, 1.0f));
        if (ImGui::Button("Download & Install", ImVec2(180.0f, 30.0f))) {
            updater.DownloadAndInstall();
        }
        ImGui::PopStyleColor(2);
    }

    if (isBusy) ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Preferences & channel
    if (ImGui::Checkbox("Check for updates automatically on startup", &s.checkUpdatesOnStartup)) {
        sm.Save();
    }

    ImGui::Spacing();
    ImGui::Text("Update Channel:");
    const char* channels[] = { "stable", "beta" };
    const char* labels[] = { "Stable", "Beta" };
    int curCh = (s.updateChannel == "beta") ? 1 : 0;

    for (int i = 0; i < 2; ++i) {
        if (i > 0) ImGui::SameLine();
        bool active = (curCh == i);
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ToVec4(theme.surface2));

        if (ImGui::Button(labels[i], ImVec2(90.0f, 26.0f))) {
            s.updateChannel = channels[i];
            sm.Save();
        }

        if (active) ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // GitHub Repository Configuration
    ImGui::TextColored(ToVec4(theme.accent), "GitHub Repository Configuration:");
    ImGui::TextColored(ToVec4(theme.textDim), "Allows downloading releases from a repository with optional authentication.");
    ImGui::Spacing();

    ImGui::Text("Repository (owner/repo):");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText("##RepoInput", m_RepoBuffer, sizeof(m_RepoBuffer))) {
        s.githubRepo = m_RepoBuffer;
        sm.Save();
    }

    ImGui::Spacing();

    ImGui::Text("GitHub Token (PAT for private repository):");
    float eyeBtnW = 40.0f;
    float tokenInputW = ImGui::GetContentRegionAvail().x - eyeBtnW - ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetNextItemWidth(tokenInputW);

    ImGuiInputTextFlags tokenFlags = m_ShowToken ? 0 : ImGuiInputTextFlags_Password;
    if (ImGui::InputText("##TokenInput", m_TokenBuffer, sizeof(m_TokenBuffer), tokenFlags)) {
        s.githubToken = m_TokenBuffer;
        sm.Save();
    }

    ImGui::SameLine();
    if (ImGui::Button(m_ShowToken ? "Hide" : "Show", ImVec2(eyeBtnW + 35.0f, 0))) {
        m_ShowToken = !m_ShowToken;
    }

    ImGui::TextColored(ToVec4(theme.textFaint),
        "Generate a Personal Access Token on GitHub with 'repo' or 'contents:read' permission.\n"
        "The token is stored locally and securely on your device to query and download releases.");
}

} // namespace WitcherNotes::UI
