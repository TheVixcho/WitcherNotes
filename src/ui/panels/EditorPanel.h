#pragma once

#include <imgui.h>
#include <string>

namespace WitcherNotes::UI {

class EditorPanel {
public:
    EditorPanel();
    void Render();

    bool IsReadingMode() const { return m_ReadingMode; }
    void SetReadingMode(bool rm) { m_ReadingMode = rm; }

private:
    void RenderToolbar();
    void InsertSnippet(const std::string& snippet);

    char m_TitleBuffer[256] = {};
    std::string m_ContentBuffer;
    int m_LastLoadedPageIdx = -1;
    int m_LastLoadedChapterIdx = -1;
    int m_LastLoadedBookIdx = -1;
    bool m_ReadingMode = false;
};

} // namespace WitcherNotes::UI
