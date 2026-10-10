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

    // Título de la sección
    ImGui::TextColored(ToVec4(theme.accentLight), "📚 BIBLIOTECA DE DOCUMENTOS Y CUADERNOS");
    ImGui::TextColored(ToVec4(theme.textDim), "Selecciona un documento para abrirlo y comenzar a editar o crea uno nuevo.");
    ImGui::Spacing();

    // Buscador y Botón Nuevo
    float availW = ImGui::GetContentRegionAvail().x;
    float btnWidth = 190.0f;
    bool hasQuery = (m_SearchQuery[0] != '\0');
    float clearBtnWidth = hasQuery ? 80.0f : 0.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;

    float searchWidth = availW - btnWidth - spacing - (hasQuery ? (clearBtnWidth + spacing) : 0.0f) - 4.0f;
    if (searchWidth < 180.0f) searchWidth = 180.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));

    ImGui::SetNextItemWidth(searchWidth);
    ImGui::InputTextWithHint("##SearchInput", "🔍 Buscar en todos los cuadernos y hojas...", m_SearchQuery, sizeof(m_SearchQuery));

    if (hasQuery) {
        ImGui::SameLine();
        if (ImGui::Button("✕ Limpiar", ImVec2(clearBtnWidth, 0.0f))) {
            m_SearchQuery[0] = '\0';
        }
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ToVec4(theme.accent));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.08f, 0.05f, 1.0f));
    if (ImGui::Button("+ Nuevo Documento", ImVec2(btnWidth, 0.0f))) {
        m_OpenNewDocModal = true;
    }
    ImGui::PopStyleColor(2);

    ImGui::PopStyleVar();
}

void LibraryPanel::RenderSearchResults() {
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();
    auto results = BookRepository::Get().Search(m_SearchQuery);

    ImGui::TextColored(ToVec4(theme.accentLight), "Resultados de búsqueda (%zu):", results.size());
    ImGui::Spacing();

    if (results.empty()) {
        ImGui::TextColored(ToVec4(theme.textDim), "No se hallaron coincidencias en ningún documento.");
        return;
    }

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        ImGui::PushID((int)i);

        ImGui::BeginChild("##ResultCard", ImVec2(ImGui::GetContentRegionAvail().x, 70.0f), true);
        ImGui::TextColored(ToVec4(theme.accent), "📄 %s", r.pageTitle.c_str());
        ImGui::SameLine();
        ImGui::TextColored(ToVec4(theme.textDim), "(Documento: %s > %s)", r.bookTitle.c_str(), r.chapterTitle.c_str());

        ImGui::TextColored(ToVec4(theme.textFaint), "   %s", r.matchSnippet.c_str());

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 130.0f);
        if (ImGui::Button("Abrir Hoja", ImVec2(120.0f, 24.0f))) {
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

            // Tarjeta de documento (ancho 0.0f = fill column)
            ImVec2 cardSize(0.0f, cardH);
            bool cardClicked = false;

            ImGui::BeginChild("##DocCard", cardSize, true, ImGuiWindowFlags_NoScrollbar);

            // Título del documento
            ImGui::Spacing();
            ImGui::TextColored(ToVec4(theme.accentLight), "📖 %s", b.title.c_str());

            // Subtítulo con wrapping
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (!b.subtitle.empty()) {
                ImGui::TextColored(ToVec4(theme.textDim), "%s", b.subtitle.c_str());
            } else {
                ImGui::TextColored(ToVec4(theme.textFaint), "Sin descripción adicional");
            }
            ImGui::PopTextWrapPos();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Metadatos
            ImGui::TextColored(ToVec4(theme.textDim), "Secciones: %zu  |  Hojas: %d  |  Palabras: %d",
                               b.chapters.size(), b.GetTotalPages(), b.GetTotalWords());
            ImGui::TextColored(ToVec4(theme.textFaint), "Autor: %s", b.author.c_str());

            ImGui::Spacing();
            ImGui::Spacing();

            // Botones de acción en la parte inferior de la tarjeta
            float btnH = 28.0f * fontScale;
            float totalBtnW = ImGui::GetContentRegionAvail().x;
            float optionsW = std::clamp(totalBtnW * 0.35f, 75.0f, 105.0f);
            float openW = totalBtnW - optionsW - ImGui::GetStyle().ItemSpacing.x;

            ImGui::PushStyleColor(ImGuiCol_Button, ToVec4(theme.surface1));
            if (ImGui::Button("Abrir Documento", ImVec2(openW, btnH))) {
                cardClicked = true;
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();

            // Opciones
            if (ImGui::Button("⋯ Opciones", ImVec2(optionsW, btnH))) {
                ImGui::OpenPopup("##DocOptionsPopup");
            }

            if (ImGui::BeginPopup("##DocOptionsPopup")) {
                if (ImGui::MenuItem("Abrir")) {
                    cardClicked = true;
                }
                if (ImGui::MenuItem("Renombrar...")) {
                    m_DocToRename = i;
                    strncpy(m_RenameDocTitle, b.title.c_str(), sizeof(m_RenameDocTitle) - 1);
                    m_RenameDocTitle[sizeof(m_RenameDocTitle) - 1] = '\0';
                    m_OpenRenameDocModal = true;
                }
                if (ImGui::BeginMenu("Exportar Documento...")) {
                    if (ImGui::MenuItem("Libro Digital EPUB (.epub)")) {
                        std::string out;
                        if (ExportManager::ExportBook(b, ExportFormat::EPUB, out)) {
                            m_ExportStatusMsg = "Documento exportado exitosamente a EPUB";
                            m_LastExportedFile = out;
                        } else {
                            m_ExportStatusMsg = "Error al exportar el documento a EPUB.";
                            m_LastExportedFile = "";
                        }
                        m_ShowExportModal = true;
                    }
                    if (ImGui::MenuItem("Documento PDF (.pdf)")) {
                        std::string out;
                        if (ExportManager::ExportBook(b, ExportFormat::PDF, out)) {
                            m_ExportStatusMsg = "Documento exportado exitosamente a PDF";
                            m_LastExportedFile = out;
                        } else {
                            m_ExportStatusMsg = "Error al exportar el documento a PDF.";
                            m_LastExportedFile = "";
                        }
                        m_ShowExportModal = true;
                    }
                    if (ImGui::MenuItem("Microsoft Word DOCX (.docx)")) {
                        std::string out;
                        if (ExportManager::ExportBook(b, ExportFormat::DOCX, out)) {
                            m_ExportStatusMsg = "Documento exportado exitosamente a DOCX";
                            m_LastExportedFile = out;
                        } else {
                            m_ExportStatusMsg = "Error al exportar el documento a DOCX.";
                            m_LastExportedFile = "";
                        }
                        m_ShowExportModal = true;
                    }
                    if (ImGui::MenuItem("Markdown (.md)")) {
                        std::string out;
                        if (ExportManager::ExportBook(b, ExportFormat::Markdown, out)) {
                            m_ExportStatusMsg = "Documento exportado exitosamente a Markdown";
                            m_LastExportedFile = out;
                        } else {
                            m_ExportStatusMsg = "Error al exportar el documento a Markdown.";
                            m_LastExportedFile = "";
                        }
                        m_ShowExportModal = true;
                    }
                    ImGui::EndMenu();
                }
                if (books.size() > 1) {
                    ImGui::Separator();
                    if (ImGui::MenuItem("Eliminar Documento")) {
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

        // Tarjeta para crear nuevo documento al final
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
        const char* labelText = "Crear Nuevo Documento";
        const char* subText = "Comenzar un cuaderno en blanco";

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
        if (ImGui::Button("Crear Cuaderno", ImVec2(btnW, 30.0f * fontScale))) {
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
        ImGui::OpenPopup("Crear Nuevo Documento");
        m_OpenNewDocModal = false;
    }

    if (ImGui::BeginPopupModal("Crear Nuevo Documento", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Configura los detalles del nuevo cuaderno o documento:");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::InputText("Título", m_NewDocTitle, sizeof(m_NewDocTitle));
        ImGui::InputText("Descripción", m_NewDocSubtitle, sizeof(m_NewDocSubtitle));
        ImGui::InputText("Autor", m_NewDocAuthor, sizeof(m_NewDocAuthor));
        ImGui::Spacing();

        if (ImGui::Button("Crear y Abrir", ImVec2(140, 32))) {
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
        if (ImGui::Button("Cancelar", ImVec2(90, 32))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void LibraryPanel::RenderRenameDocumentModal() {
    if (m_OpenRenameDocModal) {
        ImGui::OpenPopup("Renombrar Documento");
        m_OpenRenameDocModal = false;
    }

    if (ImGui::BeginPopupModal("Renombrar Documento", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Escribe el nuevo título para este documento:");
        ImGui::Spacing();

        ImGui::InputText("Nuevo Título", m_RenameDocTitle, sizeof(m_RenameDocTitle));
        ImGui::Spacing();

        if (ImGui::Button("Guardar", ImVec2(100, 30))) {
            auto& repo = BookRepository::Get();
            auto& books = repo.GetBooks();
            if (m_DocToRename >= 0 && m_DocToRename < (int)books.size()) {
                books[m_DocToRename].title = m_RenameDocTitle;
                repo.MarkDirty();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancelar", ImVec2(90, 30))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void LibraryPanel::RenderExportModal() {
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    if (m_ShowExportModal) {
        ImGui::OpenPopup("Resultado de Exportación##LibModal");
        m_ShowExportModal = false;
    }

    if (ImGui::BeginPopupModal("Resultado de Exportación##LibModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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
