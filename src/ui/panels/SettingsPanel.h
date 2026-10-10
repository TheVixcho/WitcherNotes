#pragma once

#include <imgui.h>

namespace WitcherNotes::UI {

class SettingsPanel {
public:
    SettingsPanel();
    void Render();
    void RenderCategoryUpdates();

private:
    char m_RepoBuffer[128] = {};
    char m_TokenBuffer[256] = {};
    bool m_ShowToken = false;
    bool m_BuffersInitialized = false;
};

} // namespace WitcherNotes::UI
