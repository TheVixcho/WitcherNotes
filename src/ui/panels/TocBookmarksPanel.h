#pragma once

#include <imgui.h>
#include <string>

namespace WitcherNotes::UI {

class TocBookmarksPanel {
public:
    TocBookmarksPanel();
    void Render();

private:
    void RenderNotesSection();
    void RenderBookmarksSection();
    void RenderInfoSection();

    char m_MarginaliaBuffer[2048] = {};
    int m_LastPageIdx = -1;
    int m_LastChapterIdx = -1;
    int m_LastBookIdx = -1;

    std::string m_ExportStatusMsg;
    std::string m_LastExportedFile;
    bool m_ShowExportModal = false;
};

} // namespace WitcherNotes::UI
