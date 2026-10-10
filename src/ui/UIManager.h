#pragma once

#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include "panels/LibraryPanel.h"
#include "panels/ReaderPanel.h"
#include "panels/EditorPanel.h"
#include "panels/TocBookmarksPanel.h"
#include "panels/SettingsPanel.h"

namespace WitcherNotes::UI {

enum class AppScreen {
    Library = 0,
    DocumentWorkspace
};

class UIManager {
public:
    UIManager();
    ~UIManager();

    bool Initialize(GLFWwindow* window);
    void RenderAll();
    void Shutdown();

    void SwitchToScreen(AppScreen screen) { m_CurrentScreen = screen; }
    AppScreen GetCurrentScreen() const { return m_CurrentScreen; }

private:
    void RenderLibraryView();
    void RenderWorkspaceView();

    void RenderWorkspaceHeader();
    void RenderSheetsColumn(float width, float height);
    void RenderStatusBar();

    void RenderAboutModal();
    void RenderShortcutsModal();
    void RenderExportModal();
    void RenderGlobalUpdatePopup();
    void RenderSettingsModal();

    GLFWwindow* m_Window = nullptr;
    AppScreen m_CurrentScreen = AppScreen::Library;

    std::unique_ptr<LibraryPanel> m_LibraryPanel;
    std::unique_ptr<ReaderPanel> m_ReaderPanel;
    std::unique_ptr<EditorPanel> m_EditorPanel;
    std::unique_ptr<TocBookmarksPanel> m_TocBookmarksPanel;
    std::unique_ptr<SettingsPanel> m_SettingsPanel;

    bool m_ShowAboutModal = false;
    bool m_ShowShortcutsModal = false;
    bool m_ShowExportModal = false;
    bool m_ShowSettingsModal = false;

    std::string m_ExportStatusMsg;
    std::string m_LastExportedFile;
    double m_LastAutoSaveTime = 0.0;

    char m_SheetFilter[64] = {};
    bool m_OpenNewSheetModal = false;
    char m_NewSheetTitle[128] = "Nueva Hoja";
    bool m_OpenNewSectionModal = false;
    char m_NewSectionTitle[128] = "Nueva Sección";
};

} // namespace WitcherNotes::UI
