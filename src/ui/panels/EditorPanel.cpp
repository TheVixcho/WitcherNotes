#include "EditorPanel.h"
#include "core/BookModel.h"
#include "core/SettingsManager.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <cstring>
#include <algorithm>

namespace WitcherNotes::UI {

using namespace WitcherNotes::Core;
using namespace WitcherNotes::Settings;

static ImVec4 ToVec4(const float c[4], float alpha = -1.0f) {
    float a = (alpha >= 0.0f) ? alpha : c[3];
    return ImVec4(c[0], c[1], c[2], a);
}

EditorPanel::EditorPanel() {}

void EditorPanel::Render() {
    auto& repo = BookRepository::Get();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    Book* book = repo.GetActiveBook();
    Chapter* ch = repo.GetActiveChapter();
    Page* page = repo.GetActivePage();

    if (!book || !ch || !page) {
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.4f);
        ImGui::TextColored(ToVec4(theme.textDim), "      Select or create a page in the left sidebar to begin writing.");
        return;
    }

    // Synchronize buffers if active page changed
    int curB = repo.GetActiveBookIndex();
    int curC = repo.GetActiveChapterIndex();
    int curP = repo.GetActivePageIndex();

    if (curB != m_LastLoadedBookIdx || curC != m_LastLoadedChapterIdx || curP != m_LastLoadedPageIdx) {
        strncpy(m_TitleBuffer, page->title.c_str(), sizeof(m_TitleBuffer) - 1);
        m_TitleBuffer[sizeof(m_TitleBuffer) - 1] = '\0';
        m_ContentBuffer = page->content;

        m_LastLoadedBookIdx = curB;
        m_LastLoadedChapterIdx = curC;
        m_LastLoadedPageIdx = curP;
    }

    // Header: Page title
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

    // Title input field
    float titleWidth = ImGui::GetContentRegionAvail().x - 170.0f;
    if (titleWidth < 180.0f) titleWidth = 180.0f;

    ImGui::SetNextItemWidth(titleWidth);
    if (ImGui::InputText("##PageTitle", m_TitleBuffer, sizeof(m_TitleBuffer))) {
        page->title = m_TitleBuffer;
        repo.MarkDirty();
    }

    ImGui::SameLine();

    // Toggle Edit Mode / Reading View
    if (ImGui::Button(m_ReadingMode ? "Edit Mode" : "Reading View", ImVec2(160.0f, 0))) {
        m_ReadingMode = !m_ReadingMode;
    }

    ImGui::PopStyleVar();

    ImGui::Spacing();
    RenderToolbar();
    ImGui::Spacing();

    // Main Content Area
    float availH = ImGui::GetContentRegionAvail().y - 28.0f;
    if (availH < 120.0f) availH = 120.0f;

    if (!m_ReadingMode) {
        // Edit Mode
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ToVec4(theme.surface0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 12));

        ImVec2 contentSize(ImGui::GetContentRegionAvail().x, availH);
        if (ImGui::InputTextMultiline("##PageContent", &m_ContentBuffer, contentSize,
                                      ImGuiInputTextFlags_AllowTabInput)) {
            page->content = m_ContentBuffer;
            repo.MarkDirty();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    } else {
        // Formatted Reading View
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ToVec4(theme.pageBg));
        ImGui::BeginChild("##ReaderView", ImVec2(ImGui::GetContentRegionAvail().x, availH), true);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 20));

        ImGui::TextColored(ToVec4(theme.accent), "# %s", page->title.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x - 20.0f);
        ImGui::TextColored(ToVec4(theme.pageText), "%s", page->content.c_str());
        ImGui::PopTextWrapPos();

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    // Bottom statistics
    ImGui::Spacing();
    int words = page->GetWordCount();
    int readTime = page->GetReadingTimeMinutes();
    ImGui::TextColored(ToVec4(theme.textDim), "%d words  |  %zu characters  |  ~%d min read",
                       words, m_ContentBuffer.size(), readTime);
}

void EditorPanel::RenderToolbar() {
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

    if (ImGui::Button("H1"))          InsertSnippet("# ");
    ImGui::SameLine();
    if (ImGui::Button("H2"))          InsertSnippet("## ");
    ImGui::SameLine();
    if (ImGui::Button("H3"))          InsertSnippet("### ");
    ImGui::SameLine();
    if (ImGui::Button("Bold"))        InsertSnippet("**bold text**");
    ImGui::SameLine();
    if (ImGui::Button("Italic"))      InsertSnippet("*italic text*");
    ImGui::SameLine();
    if (ImGui::Button("Quote"))       InsertSnippet("> Blockquote or key note...\n");
    ImGui::SameLine();
    if (ImGui::Button("Code"))        InsertSnippet("```\n// code snippet\n```\n");
    ImGui::SameLine();
    if (ImGui::Button("List"))        InsertSnippet("* Item\n");
    ImGui::SameLine();
    if (ImGui::Button("Numbered"))    InsertSnippet("1. Item\n");
    ImGui::SameLine();
    if (ImGui::Button("Rule"))        InsertSnippet("\n---\n\n");

    ImGui::PopStyleVar(2);
    ImGui::Separator();
}

void EditorPanel::InsertSnippet(const std::string& snippet) {
    auto& repo = BookRepository::Get();
    Page* page = repo.GetActivePage();
    if (!page) return;

    m_ContentBuffer += snippet;
    page->content = m_ContentBuffer;
    repo.MarkDirty();
}

} // namespace WitcherNotes::UI
