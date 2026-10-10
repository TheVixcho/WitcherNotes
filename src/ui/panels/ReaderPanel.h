#pragma once

#include <imgui.h>
#include <string>

namespace WitcherNotes::UI {

class ReaderPanel {
public:
    ReaderPanel();
    void Render();

    void SetFontScale(float scale) { m_FontScale = scale; }
    float GetFontScale() const { return m_FontScale; }

private:
    void RenderTwoPageSpread(float availWidth, float availHeight);
    void RenderSinglePage(float availWidth, float availHeight);
    void RenderContinuousScroll(float availWidth, float availHeight);

    void RenderPageContent(const std::string& title, const std::string& content,
                           const std::string& marginalia, const std::string& signTag,
                           int pageNumber, int totalPages,
                           const ImVec2& pos, const ImVec2& size, bool isLeftPage);

    void RenderDropCapParagraph(const std::string& text, ImDrawList* dl, ImVec2 startPos, float maxWidth);
    void RenderControlsBar();

    float m_FontScale = 1.0f;
};

} // namespace WitcherNotes::UI
