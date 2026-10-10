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
        ImGui::TextColored(ToVec4(theme.textDim), "No hay ningún documento activo.");
        return;
    }

    if (ImGui::BeginTabBar("##RightColumnTabs")) {
        // Pestaña 1: Notas y Apuntes
        if (ImGui::BeginTabItem("✍ Notas")) {
            RenderNotesSection();
            ImGui::EndTabItem();
        }

        // Pestaña 2: Marcadores
        if (ImGui::BeginTabItem("★ Marcadores")) {
            RenderBookmarksSection();
            ImGui::EndTabItem();
        }

        // Pestaña 3: Información
        if (ImGui::BeginTabItem("ℹ Info")) {
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
        ImGui::TextColored(ToVec4(theme.textDim), "Selecciona una hoja para agregar notas.");
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
    ImGui::TextColored(ToVec4(theme.accentLight), "✍ Notas al Margen / Apuntes Rápidos");
    ImGui::TextColored(ToVec4(theme.textDim), "Anotaciones asociadas a: '%s'", page->title.c_str());
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
    ImGui::TextColored(ToVec4(theme.accentLight), "★ Páginas Marcadas");
    ImGui::Spacing();

    // Botón para alternar marcador de la página activa
    if (page) {
        if (page->isBookmarked) {
            ImGui::PushStyleColor(ImGuiCol_Button, ToVec4(theme.accent));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.08f, 0.05f, 1.0f));
        }
        if (ImGui::Button(page->isBookmarked ? "★ Quitar Marcador de esta hoja" : "☆ Marcar esta hoja como favorita",
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

    // Lista de todas las páginas marcadas
    int bookmarkCount = 0;
    for (int c = 0; c < (int)book->chapters.size(); ++c) {
        const auto& ch = book->chapters[c];
        for (int p = 0; p < (int)ch.pages.size(); ++p) {
            const auto& pg = ch.pages[p];
            if (pg.isBookmarked) {
                bookmarkCount++;
                int globalPg = book->GetGlobalPageIndex(c, p) + 1;

                ImGui::PushID(bookmarkCount);
                std::string label = "★ Pág. " + std::to_string(globalPg) + ": " + pg.title;

                bool isCurrent = (c == repo.GetActiveChapterIndex() && p == repo.GetActivePageIndex());
                if (ImGui::Selectable(label.c_str(), isCurrent)) {
                    repo.SetActiveIndices(repo.GetActiveBookIndex(), c, p);
                }
                ImGui::TextColored(ToVec4(theme.textFaint), "    Sección: %s", ch.title.c_str());
                ImGui::Spacing();
                ImGui::PopID();
            }
        }
    }

    if (bookmarkCount == 0) {
        ImGui::TextColored(ToVec4(theme.textDim), "No tienes páginas marcadas todavía.");
        ImGui::TextColored(ToVec4(theme.textFaint), "Usa el botón de arriba para fijar accesos directos a hojas importantes.");
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
    ImGui::TextColored(ToVec4(theme.accentLight), "ℹ INFORMACIÓN DEL DOCUMENTO");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Categoría / Etiqueta de la hoja actual
    ImGui::TextColored(ToVec4(theme.textDim), "Etiqueta de la Hoja:");
    const char* categories[] = {
        "GENERAL", "IDEAS", "DRAFT", "IMPORTANT", "REFERENCE", "TASKS", "PROJECT", "STUDY"
    };
    const char* displayNames[] = {
        "Nota General", "Ideas y Bocetos", "Borrador", "Prioritario / Importante",
        "Referencia", "Tareas / To-Do", "Proyecto", "Estudio"
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

    // Estadísticas
    ImGui::TextColored(ToVec4(theme.textPrimary), "Documento: %s", book->title.c_str());
    ImGui::TextColored(ToVec4(theme.textDim), "Autor: %s", book->author.c_str());
    ImGui::TextColored(ToVec4(theme.textDim), "Sección actual: %s", ch ? ch->title.c_str() : "-");
    ImGui::TextColored(ToVec4(theme.textDim), "Total de secciones: %zu", book->chapters.size());
    ImGui::TextColored(ToVec4(theme.textDim), "Total de hojas: %d", book->GetTotalPages());
    ImGui::TextColored(ToVec4(theme.textDim), "Palabras del documento: %d", book->GetTotalWords());
    ImGui::TextColored(ToVec4(theme.textFaint), "Creado: %s", page->createdAt.c_str());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Exportación Multi-formato
    ImGui::TextColored(ToVec4(theme.accent), "Exportar Hoja Actual:");
    float halfBtn = (ImGui::GetContentRegionAvail().x - 6.0f) * 0.5f;

    if (ImGui::Button("📄 PDF", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportCurrentPage(*book, *page, ExportFormat::PDF, out)) {
            m_ExportStatusMsg = "Hoja exportada exitosamente a PDF";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Error al exportar la hoja a PDF.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("📚 EPUB", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportCurrentPage(*book, *page, ExportFormat::EPUB, out)) {
            m_ExportStatusMsg = "Hoja exportada exitosamente a EPUB";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Error al exportar la hoja a EPUB.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }

    if (ImGui::Button("📝 Word (DOCX)", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportCurrentPage(*book, *page, ExportFormat::DOCX, out)) {
            m_ExportStatusMsg = "Hoja exportada exitosamente a DOCX";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Error al exportar la hoja a DOCX.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("📃 Markdown", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportCurrentPage(*book, *page, ExportFormat::Markdown, out)) {
            m_ExportStatusMsg = "Hoja exportada exitosamente a Markdown";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Error al exportar la hoja a Markdown.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ToVec4(theme.accent), "Exportar Documento Completo:");
    if (ImGui::Button("📄 Cuaderno a PDF", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportBook(*book, ExportFormat::PDF, out)) {
            m_ExportStatusMsg = "Cuaderno exportado exitosamente a PDF";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Error al exportar el cuaderno a PDF.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("📚 Cuaderno a EPUB", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportBook(*book, ExportFormat::EPUB, out)) {
            m_ExportStatusMsg = "Cuaderno exportado exitosamente a EPUB";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Error al exportar el cuaderno a EPUB.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }

    if (ImGui::Button("📝 Cuaderno a DOCX", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportBook(*book, ExportFormat::DOCX, out)) {
            m_ExportStatusMsg = "Cuaderno exportado exitosamente a DOCX";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Error al exportar el cuaderno a DOCX.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("📃 Cuaderno a MD", ImVec2(halfBtn, 28.0f))) {
        std::string out;
        if (ExportManager::ExportBook(*book, ExportFormat::Markdown, out)) {
            m_ExportStatusMsg = "Cuaderno exportado exitosamente a Markdown";
            m_LastExportedFile = out;
        } else {
            m_ExportStatusMsg = "Error al exportar el cuaderno a Markdown.";
            m_LastExportedFile = "";
        }
        m_ShowExportModal = true;
    }

    ImGui::Spacing();
    if (ImGui::Button("📁 Abrir Carpeta de Exportaciones", ImVec2(ImGui::GetContentRegionAvail().x, 26.0f))) {
        ExportManager::OpenExportFolder(AppPaths::GetBackupsDir());
    }

    // Modal de estado de exportación
    if (m_ShowExportModal) {
        ImGui::OpenPopup("Resultado de Exportación##InfoModal");
        m_ShowExportModal = false;
    }

    if (ImGui::BeginPopupModal("Resultado de Exportación##InfoModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ToVec4(theme.accentLight), "%s", m_ExportStatusMsg.c_str());
        if (!m_LastExportedFile.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ToVec4(theme.textDim), "Archivo guardado en:");
            ImGui::TextColored(ToVec4(theme.textPrimary), "%s", m_LastExportedFile.c_str());
            ImGui::Spacing();
            if (ImGui::Button("📁 Abrir Carpeta", ImVec2(130, 30))) {
                ExportManager::OpenExportFolder(m_LastExportedFile);
            }
            ImGui::SameLine();
        }
        if (ImGui::Button("Aceptar", ImVec2(100, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace WitcherNotes::UI
