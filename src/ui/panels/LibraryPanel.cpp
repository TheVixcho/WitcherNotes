#include "LibraryPanel.h"
#include "core/BookModel.h"
#include "core/SettingsManager.h"
#include "core/AppPaths.h"
#include "core/ExportManager.h"
#include <imgui.h>
#include <algorithm>
#include <cstring>

namespace WitcherNotes::UI {

using namespace WitcherNotes::Core;
using namespace WitcherNotes::Settings;

static ImVec4 ToVec4(const float c[4], float alpha = -1.0f) {
    float a = (alpha >= 0.0f) ? alpha : c[3];
    return ImVec4(c[0], c[1], c[2], a);
}

LibraryPanel::LibraryPanel() {}

void LibraryPanel::Render() {
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    RenderTopBar();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Área scrolleable para las tarjetas
    float availH = ImGui::GetContentRegionAvail().y;
    ImGui::BeginChild("##LibraryCardsScroll", ImVec2(0.0f, availH), false);

    std::string q = m_SearchQuery;
    if (!q.empty()) {
        RenderSearchResults();
    } else {
        RenderDocumentGrid();
    }

    ImGui::EndChild();

    RenderNewDocumentModal();
    RenderRenameDocumentModal();
    RenderExportModal();
}

void LibraryPanel::RenderTopBar() {
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    // Section title
    ImGui::TextColored(ToVec4(theme.accentLight), "DOCUMENT & NOTEBOOK LIBRARY");
    ImGui::TextColored(ToVec4(theme.textDim), "Select a notebook to open and edit, or create a new one.");
    ImGui::Spacing();

    // Search input and New Notebook button
    float availW = ImGui::GetContentRegionAvail().x;
    float btnWidth = 180.0f;
    bool hasQuery = (m_SearchQuery[0] != '\0');
    float clearBtnWidth = hasQuery ? 75.0f : 0.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;

    float searchWidth = availW - btnWidth - spacing - (hasQuery ? (clearBtnWidth + spacing) : 0.0f) - 4.0f;
    if (searchWidth < 180.0f) searchWidth = 180.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));

    ImGui::SetNextItemWidth(searchWidth);
    ImGui::InputTextWithHint("##SearchInput", "Search in all notebooks and pages...", m_SearchQuery, sizeof(m_SearchQuery));

    if (hasQuery) {
        ImGui::SameLine();
        if (ImGui::Button("Clear", ImVec2(clearBtnWidth, 0.0f))) {
            m_SearchQuery[0] = '\0';
        }
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ToVec4(theme.accent));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.08f, 0.05f, 1.0f));
    if (ImGui::Button("+ New Notebook", ImVec2(btnWidth, 0.0f))) {
        m_OpenNewDocModal = true;
    }
    ImGui::PopStyleColor(2);

    ImGui::PopStyleVar();
}

void LibraryPanel::RenderSearchResults() {
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();
    auto results = BookRepository::Get().Search(m_SearchQuery);

    ImGui::TextColored(ToVec4(theme.accentLight), "Search results (%zu):", results.size());
    ImGui::Spacing();

    if (results.empty()) {
        ImGui::TextColored(ToVec4(theme.textDim), "No matching documents or pages found.");
        return;
    }

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        ImGui::PushID((int)i);

        ImGui::BeginChild("##ResultCard", ImVec2(ImGui::GetContentRegionAvail().x, 70.0f), true);
        ImGui::TextColored(ToVec4(theme.accent), "%s", r.pageTitle.c_str());
        ImGui::SameLine();
        ImGui::TextColored(ToVec4(theme.textDim), "(Notebook: %s > %s)", r.bookTitle.c_str(), r.chapterTitle.c_str());

        ImGui::TextColored(ToVec4(theme.textFaint), "   %s", r.matchSnippet.c_str());

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 130.0f);
        if (ImGui::Button("Open Page", ImVec2(120.0f, 24.0f))) {
            BookRepository::Get().SetActiveIndices(r.bookIdx, r.chapterIdx, r.pageIdx);
            if (m_OnOpenDocument) {
                m_OnOpenDocument(r.bookIdx);
            }
        }

        ImGui::EndChild();
        ImGui::Spacing();
        ImGui::PopID();
    }
}

void LibraryPanel::RenderDocumentGrid() {
    auto& repo = BookRepository::Get();
    auto& books = repo.GetBooks();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    float availW = ImGui::GetContentRegionAvail().x;
    int cols = (int)(availW / 310.0f);
    if (cols < 1) cols = 1;
    if (cols > 4) cols = 4;

    float fontScale = ImGui::GetFontSize() / 15.0f;
    if (fontScale < 1.0f) fontScale = 1.0f;
    float cardH = 210.0f * fontScale;

    if (ImGui::BeginTable("##BooksGrid", cols, ImGuiTableFlags_SizingStretchSame)) {
        for (int i = 0; i < (int)books.size(); ++i) {
            ImGui::TableNextColumn();
            const auto& b = books[i];

            ImGui::PushID(i);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ToVec4(theme.border));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ToVec4(theme.surface0));

            // Document card
            ImVec2 cardSize(0.0f, cardH);
            bool cardClicked = false;

            ImGui::BeginChild("##DocCard", cardSize, true, ImGuiWindowFlags_NoScrollbar);

            // Document title
            ImGui::Spacing();
            ImGui::TextColored(ToVec4(theme.accentLight), "%s", b.title.c_str());

            // Subtitle with text wrapping
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (!b.subtitle.empty()) {
                ImGui::TextColored(ToVec4(theme.textDim), "%s", b.subtitle.c_str());
            } else {
                ImGui::TextColored(ToVec4(theme.textFaint), "No description provided");
            }
            ImGui::PopTextWrapPos();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Metadata
            ImGui::TextColored(ToVec4(theme.textDim), "Sections: %zu  |  Pages: %d  |  Words: %d",
                               b.chapters.size(), b.GetTotalPages(), b.GetTotalWords());
            ImGui::TextColored(ToVec4(theme.textFaint), "Author: %s", b.author.c_str());

            ImGui::Spacing();
            ImGui::Spacing();

            // Action buttons
            float btnH = 28.0f * fontScale;
            float totalBtnW = ImGui::GetContentRegionAvail().x;
            float optionsW = std::clamp(totalBtnW * 0.35f, 75.0f, 105.0f);
            float openW = totalBtnW - optionsW - ImGui::GetStyle().ItemSpacing.x;

            ImGui::PushStyleColor(ImGuiCol_Button, ToVec4(theme.surface1));
            if (ImGui::Button("Open Notebook", ImVec2(openW, btnH))) {
                cardClicked = true;
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();

            // Options popup
            if (ImGui::Button("Options", ImVec2(optionsW, btnH))) {
                ImGui::OpenPopup("##DocOptionsPopup");
            }

            if (ImGui::BeginPopup("##DocOptionsPopup")) {
                if (ImGui::MenuItem("Open")) {
                    cardClicked = true;
                }
                if (ImGui::MenuItem("Rename...")) {
                    m_DocToRename = i;
                    strncpy(m_RenameDocTitle, b.title.c_str(), sizeof(m_RenameDocTitle) - 1);
                    m_RenameDocTitle[sizeof(m_RenameDocTitle) - 1] = '\0';
                    m_OpenRenameDocModal = true;
                }
                if (ImGui::BeginMenu("Export Notebook...")) {
                    if (ImGui::MenuItem("Digital Book (.epub)")) {
                        std::string out;
                        if (ExportManager::ExportBook(b, ExportFormat::EPUB, out)) {
                            m_ExportStatusMsg = "Notebook exported successfully to EPUB";
                            m_LastExportedFile = out;
                        } else {
                            m_ExportStatusMsg = "Failed to export notebook to EPUB.";
                            m_LastExportedFile = "";
                        }
                        m_ShowExportModal = true;
                    }
                    if (ImGui::MenuItem("PDF Document (.pdf)")) {
                        std::string out;
                        if (ExportManager::ExportBook(b, ExportFormat::PDF, out)) {
                            m_ExportStatusMsg = "Notebook exported successfully to PDF";
                            m_LastExportedFile = out;
                        } else {
                            m_ExportStatusMsg = "Failed to export notebook to PDF.";
                            m_LastExportedFile = "";
                        }
                        m_ShowExportModal = true;
                    }
                    if (ImGui::MenuItem("Microsoft Word (.docx)")) {
                        std::string out;
                        if (ExportManager::ExportBook(b, ExportFormat::DOCX, out)) {
                            m_ExportStatusMsg = "Notebook exported successfully to DOCX";
                            m_LastExportedFile = out;
                        } else {
                            m_ExportStatusMsg = "Failed to export notebook to DOCX.";
                            m_LastExportedFile = "";
                        }
                        m_ShowExportModal = true;
                    }
                    if (ImGui::MenuItem("Markdown (.md)")) {
                        std::string out;
                        if (ExportManager::ExportBook(b, ExportFormat::Markdown, out)) {
                            m_ExportStatusMsg = "Notebook exported successfully to Markdown";
                            m_LastExportedFile = out;
                        } else {
                            m_ExportStatusMsg = "Failed to export notebook to Markdown.";
                            m_LastExportedFile = "";
                        }
                        m_ShowExportModal = true;
                    }
                    ImGui::EndMenu();
                }
                if (books.size() > 1) {
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete Notebook")) {
                        repo.DeleteBook(i);
                    }
                }
                ImGui::EndPopup();
            }

            if (ImGui::IsWindowHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                cardClicked = true;
            }

            ImGui::EndChild();
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar();

            if (cardClicked) {
                repo.SetActiveBook(i);
                if (m_OnOpenDocument) {
                    m_OnOpenDocument(i);
                }
            }

            ImGui::PopID();
        }

        // New notebook card at end
        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ToVec4(theme.borderFaint));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ToVec4(theme.base));

        ImVec2 newCardSize(0.0f, cardH);
        ImGui::BeginChild("##NewDocCard", newCardSize, true, ImGuiWindowFlags_NoScrollbar);

        float centerOffset = (cardH - 95.0f) * 0.5f;
        if (centerOffset > 10.0f) ImGui::SetCursorPosY(centerOffset);

        float availInside = ImGui::GetContentRegionAvail().x;
        const char* plusText = "+";
        const char* labelText = "Create New Notebook";
        const char* subText = "Start with a clean blank notebook";

        float wPlus = ImGui::CalcTextSize(plusText).x;
        float wLabel = ImGui::CalcTextSize(labelText).x;
        float wSub = ImGui::CalcTextSize(subText).x;

        ImGui::SetCursorPosX((availInside - wPlus) * 0.5f);
        ImGui::TextColored(ToVec4(theme.accent), "%s", plusText);

        ImGui::SetCursorPosX((availInside - wLabel) * 0.5f);
        ImGui::TextColored(ToVec4(theme.textPrimary), "%s", labelText);

        ImGui::SetCursorPosX((availInside - wSub) * 0.5f);
        ImGui::TextColored(ToVec4(theme.textFaint), "%s", subText);

        ImGui::Spacing();
        float btnW = std::clamp(availInside - 40.0f, 160.0f, 220.0f);
        ImGui::SetCursorPosX((availInside - btnW) * 0.5f);
        if (ImGui::Button("Create Notebook", ImVec2(btnW, 30.0f * fontScale))) {
            m_OpenNewDocModal = true;
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_OpenNewDocModal = true;
        }

        ImGui::EndChild();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        ImGui::EndTable();
    }
}

void LibraryPanel::RenderNewDocumentModal() {
    if (m_OpenNewDocModal) {
        ImGui::OpenPopup("Create New Notebook");
        m_OpenNewDocModal = false;
    }

    if (ImGui::BeginPopupModal("Create New Notebook", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Configure details for the new notebook or document:");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::InputText("Title", m_NewDocTitle, sizeof(m_NewDocTitle));
        ImGui::InputText("Description", m_NewDocSubtitle, sizeof(m_NewDocSubtitle));
        ImGui::InputText("Author", m_NewDocAuthor, sizeof(m_NewDocAuthor));
        ImGui::Spacing();

        if (ImGui::Button("Create & Open", ImVec2(140, 32))) {
            auto& repo = BookRepository::Get();
            repo.AddBook(m_NewDocTitle, m_NewDocSubtitle, m_NewDocAuthor, "Dark");
            int newIdx = (int)repo.GetBooks().size() - 1;
            repo.SetActiveBook(newIdx);
            if (m_OnOpenDocument) {
                m_OnOpenDocument(newIdx);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 32))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void LibraryPanel::RenderRenameDocumentModal() {
    if (m_OpenRenameDocModal) {
        ImGui::OpenPopup("Rename Notebook");
        m_OpenRenameDocModal = false;
    }

    if (ImGui::BeginPopupModal("Rename Notebook", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter the new title for this notebook:");
        ImGui::Spacing();

        ImGui::InputText("New Title", m_RenameDocTitle, sizeof(m_RenameDocTitle));
        ImGui::Spacing();

        if (ImGui::Button("Save", ImVec2(100, 30))) {
            auto& repo = BookRepository::Get();
            auto& books = repo.GetBooks();
            if (m_DocToRename >= 0 && m_DocToRename < (int)books.size()) {
                books[m_DocToRename].title = m_RenameDocTitle;
                repo.MarkDirty();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 30))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void LibraryPanel::RenderExportModal() {
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    if (m_ShowExportModal) {
        ImGui::OpenPopup("Export Result##LibModal");
        m_ShowExportModal = false;
    }

    if (ImGui::BeginPopupModal("Export Result##LibModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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
