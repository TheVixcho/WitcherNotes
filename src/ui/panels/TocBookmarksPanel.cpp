#include "TocBookmarksPanel.h"
#include "core/BookModel.h"
#include "core/SettingsManager.h"
#include "core/AppPaths.h"
#include "core/ExportManager.h"
#include <imgui.h>
#include <cstring>
#include <iostream>

namespace WitcherNotes::UI {

using namespace WitcherNotes::Core;
using namespace WitcherNotes::Settings;

static ImVec4 ToVec4(const float c[4], float alpha = -1.0f) {
    float a = (alpha >= 0.0f) ? alpha : c[3];
    return ImVec4(c[0], c[1], c[2], a);
}

TocBookmarksPanel::TocBookmarksPanel() {}

void TocBookmarksPanel::Render() {
    auto& repo = BookRepository::Get();
    Book* book = repo.GetActiveBook();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    if (!book) {
        ImGui::TextColored(ToVec4(theme.textDim), "No active document.");
        return;
    }

    if (ImGui::BeginTabBar("##RightColumnTabs")) {
        // Tab 1: Notes and marginalia
        if (ImGui::BeginTabItem("Notes")) {
            RenderNotesSection();
            ImGui::EndTabItem();
        }

        // Tab 2: Bookmarks
        if (ImGui::BeginTabItem("Bookmarks")) {
            RenderBookmarksSection();
            ImGui::EndTabItem();
        }

        // Tab 3: Details & Info
        if (ImGui::BeginTabItem("Details")) {
            RenderInfoSection();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void TocBookmarksPanel::RenderNotesSection() {
    auto& repo = BookRepository::Get();
    Page* page = repo.GetActivePage();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    if (!page) {
        ImGui::TextColored(ToVec4(theme.textDim), "Select a page to add notes.");
        return;
    }

    int curB = repo.GetActiveBookIndex();
    int curC = repo.GetActiveChapterIndex();
    int curP = repo.GetActivePageIndex();

    if (curB != m_LastBookIdx || curC != m_LastChapterIdx || curP != m_LastPageIdx) {
        strncpy(m_MarginaliaBuffer, page->marginalia.c_str(), sizeof(m_MarginaliaBuffer) - 1);
        m_MarginaliaBuffer[sizeof(m_MarginaliaBuffer) - 1] = '\0';

        m_LastBookIdx = curB;
        m_LastChapterIdx = curC;
        m_LastPageIdx = curP;
    }

    ImGui::Spacing();
    ImGui::TextColored(ToVec4(theme.accentLight), "Marginalia & Quick Notes");
    ImGui::TextColored(ToVec4(theme.textDim), "Notes linked to: '%s'", page->title.c_str());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    float availH = ImGui::GetContentRegionAvail().y - 10.0f;
    if (availH < 150.0f) availH = 150.0f;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ToVec4(theme.surface0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));

    if (ImGui::InputTextMultiline("##MarginaliaInput", m_MarginaliaBuffer, sizeof(m_MarginaliaBuffer),
                                   ImVec2(ImGui::GetContentRegionAvail().x, availH))) {
        page->marginalia = m_MarginaliaBuffer;
        repo.MarkDirty();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void TocBookmarksPanel::RenderBookmarksSection() {
    auto& repo = BookRepository::Get();
    Book* book = repo.GetActiveBook();
    Page* page = repo.GetActivePage();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    if (!book) return;

    ImGui::Spacing();
    ImGui::TextColored(ToVec4(theme.accentLight), "Bookmarked Pages");
    ImGui::Spacing();

    // Toggle bookmark for active page
    if (page) {
        if (page->isBookmarked) {
            ImGui::PushStyleColor(ImGuiCol_Button, ToVec4(theme.accent));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.08f, 0.05f, 1.0f));
        }
        if (ImGui::Button(page->isBookmarked ? "Remove Bookmark" : "Bookmark this Page",
                          ImVec2(ImGui::GetContentRegionAvail().x, 32.0f))) {
            page->isBookmarked = !page->isBookmarked;
            repo.MarkDirty();
        }
        if (page->isBookmarked) {
            ImGui::PopStyleColor(2);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // List of all bookmarked pages
    int bookmarkCount = 0;
    for (int c = 0; c < (int)book->chapters.size(); ++c) {
        const auto& ch = book->chapters[c];
        for (int p = 0; p < (int)ch.pages.size(); ++p) {
            const auto& pg = ch.pages[p];
            if (pg.isBookmarked) {
                bookmarkCount++;
                int globalPg = book->GetGlobalPageIndex(c, p) + 1;

                ImGui::PushID(bookmarkCount);
                std::string label = "Page " + std::to_string(globalPg) + ": " + pg.title;

                bool isCurrent = (c == repo.GetActiveChapterIndex() && p == repo.GetActivePageIndex());
                if (ImGui::Selectable(label.c_str(), isCurrent)) {
                    repo.SetActiveIndices(repo.GetActiveBookIndex(), c, p);
                }
                ImGui::TextColored(ToVec4(theme.textFaint), "    Section: %s", ch.title.c_str());
                ImGui::Spacing();
                ImGui::PopID();
            }
        }
    }

    if (bookmarkCount == 0) {
        ImGui::TextColored(ToVec4(theme.textDim), "No pages bookmarked yet.");
        ImGui::TextColored(ToVec4(theme.textFaint), "Use the button above to bookmark important pages.");
    }
}

void TocBookmarksPanel::RenderInfoSection() {
    auto& repo = BookRepository::Get();
    Book* book = repo.GetActiveBook();
    Chapter* ch = repo.GetActiveChapter();
    Page* page = repo.GetActivePage();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    if (!book || !page) return;

    ImGui::Spacing();
    ImGui::TextColored(ToVec4(theme.accentLight), "DOCUMENT DETAILS");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Page Category tag
    ImGui::TextColored(ToVec4(theme.textDim), "Page Category:");
    const char* categories[] = {
        "GENERAL", "IDEAS", "DRAFT", "IMPORTANT", "REFERENCE", "TASKS", "PROJECT", "STUDY"
    };
    const char* displayNames[] = {
        "General Note", "Ideas & Brainstorm", "Draft", "Priority / Important",
        "Reference", "Tasks / To-Do", "Project", "Study & Research"
    };

    int curCatIdx = 0;
    for (int i = 0; i < 8; ++i) {
        if (page->signTag == categories[i]) {
            curCatIdx = i;
            break;
        }
    }

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::Combo("##CategoryCombo", &curCatIdx, displayNames, 8)) {
        page->signTag = categories[curCatIdx];
        repo.MarkDirty();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Statistics
    ImGui::TextColored(ToVec4(theme.textPrimary), "Document: %s", book->title.c_str());
    ImGui::TextColored(ToVec4(theme.textDim), "Author: %s", book->author.c_str());
    ImGui::TextColored(ToVec4(theme.textDim), "Current section: %s", ch ? ch->title.c_str() : "-");
    ImGui::TextColored(ToVec4(theme.textDim), "Total sections: %zu", book->chapters.size());
    ImGui::TextColored(ToVec4(theme.textDim), "Total pages: %d", book->GetTotalPages());
    ImGui::TextColored(ToVec4(theme.textDim), "Total words: %d", book->GetTotalWords());
    ImGui::TextColored(ToVec4(theme.textFaint), "Created: %s", page->createdAt.c_str());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Export current page
    ImGui::TextColored(ToVec4(theme.accent), "Export Current Page:");
    float halfBtn = (ImGui::GetContentRegionAvail().x - 6.0f) * 0.5f;

    if (ImGui::Button("PDF", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportCurrentPage(*book, *page, ExportFormat::PDF, out)) {
            m_ExportStatusMsg = "Page exported successfully to PDF";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Failed to export page to PDF.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("EPUB", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportCurrentPage(*book, *page, ExportFormat::EPUB, out)) {
            m_ExportStatusMsg = "Page exported successfully to EPUB";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Failed to export page to EPUB.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }

    if (ImGui::Button("Word (DOCX)", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportCurrentPage(*book, *page, ExportFormat::DOCX, out)) {
            m_ExportStatusMsg = "Page exported successfully to DOCX";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Failed to export page to DOCX.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Markdown", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportCurrentPage(*book, *page, ExportFormat::Markdown, out)) {
            m_ExportStatusMsg = "Page exported successfully to Markdown";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Failed to export page to Markdown.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ToVec4(theme.accent), "Export Entire Notebook:");
    if (ImGui::Button("Notebook to PDF", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportBook(*book, ExportFormat::PDF, out)) {
            m_ExportStatusMsg = "Notebook exported successfully to PDF";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Failed to export notebook to PDF.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Notebook to EPUB", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportBook(*book, ExportFormat::EPUB, out)) {
            m_ExportStatusMsg = "Notebook exported successfully to EPUB";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Failed to export notebook to EPUB.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }

    if (ImGui::Button("Notebook to DOCX", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportBook(*book, ExportFormat::DOCX, out)) {
            m_ExportStatusMsg = "Notebook exported successfully to DOCX";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Failed to export notebook to DOCX.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Notebook to MD", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportBook(*book, ExportFormat::Markdown, out)) {
            m_ExportStatusMsg = "Notebook exported successfully to Markdown";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Failed to export notebook to Markdown.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }

    ImGui::Spacing();
    if (ImGui::Button("Open Exports Folder", ImVec2(ImGui::GetContentRegionAvail().x, 26.0f))) {
        ExportManager::OpenExportFolder(AppPaths::GetBackupsDir());
    }

    // Export status modal
    if (m_ShowExportModal) {
        ImGui::OpenPopup("Export Result##InfoModal");
        m_ShowExportModal = false;
    }

    if (ImGui::BeginPopupModal("Export Result##InfoModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ToVec4(theme.accentLight), "%s", m_ExportStatusMsg.c_str());
        if (!m_LastExportedFile.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ToVec4(theme.textDim), "File saved to:");
            ImGui::TextColored(ToVec4(theme.textPrimary), "%s", m_LastExportedFile.c_str());
            ImGui::Spacing();
            if (ImGui::Button("Open Folder", ImVec2(130, 30))) {
                ExportManager::OpenExportFolder(m_LastExportedFile);
            }
            ImGui::SameLine();
        }
        if (ImGui::Button("OK", ImVec2(100, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace WitcherNotes::UI
