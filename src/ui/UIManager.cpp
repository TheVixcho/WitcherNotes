#include "UIManager.h"
#include "core/BookModel.h"
#include "core/SettingsManager.h"
#include "core/AppPaths.h"
#include "core/ExportManager.h"
#include "core/UpdateManager.h"
#include "Version.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <cstring>
#include <algorithm>

namespace WitcherNotes::UI {

using namespace WitcherNotes::Core;
using namespace WitcherNotes::Settings;

static ImVec4 ToVec4(const float c[4], float alpha = -1.0f) {
    float a = (alpha >= 0.0f) ? alpha : c[3];
    return ImVec4(c[0], c[1], c[2], a);
}

UIManager::UIManager() {
    m_LibraryPanel      = std::make_unique<LibraryPanel>();
    m_ReaderPanel       = std::make_unique<ReaderPanel>();
    m_EditorPanel       = std::make_unique<EditorPanel>();
    m_TocBookmarksPanel = std::make_unique<TocBookmarksPanel>();
    m_SettingsPanel     = std::make_unique<SettingsPanel>();

    // Al seleccionar un documento en la biblioteca, abrir el espacio de trabajo
    m_LibraryPanel->SetOnOpenDocument([this](int) {
        m_CurrentScreen = AppScreen::DocumentWorkspace;
    });
}

UIManager::~UIManager() = default;

bool UIManager::Initialize(GLFWwindow* window) {
    m_Window = window;
    m_LastAutoSaveTime = glfwGetTime();
    UpdateManager::Get().CheckOnStartup();
    return true;
}

void UIManager::Shutdown() {
    BookRepository::Get().Save();
    SettingsManager::Get().Save();
}

void UIManager::RenderAll() {
    auto& sm = SettingsManager::Get();
    auto& s = sm.GetSettings();
    auto& repo = BookRepository::Get();
    ThemeColors theme = sm.GetCurrentThemeColors();

    // Auto-guardado periódico
    if (s.autoSave) {
        double now = glfwGetTime();
        if (now - m_LastAutoSaveTime > s.autoSaveIntervalSec) {
            if (repo.IsDirty()) {
                repo.Save();
            }
            m_LastAutoSaveTime = now;
        }
    }

    // Atajos globales
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        repo.Save();
        sm.Save();
    }
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
        if (m_CurrentScreen == AppScreen::DocumentWorkspace) {
            repo.AddPage("New Page", "GENERAL");
        }
    }
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_B)) {
        m_CurrentScreen = (m_CurrentScreen == AppScreen::Library) ?
                           AppScreen::DocumentWorkspace : AppScreen::Library;
    }
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Comma)) {
        m_ShowSettingsModal = true;
    }

    // Ventana raíz que abarca todo el viewport sin bordes
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags rootFlags = ImGuiWindowFlags_NoTitleBar
                               | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
                               | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
                               | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 8.0f));

    ImGui::Begin("##RootHostWindow", nullptr, rootFlags);
    ImGui::PopStyleVar(3);

    // Renderizar barra de menús superior
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Notebook Library", "Ctrl+B", m_CurrentScreen == AppScreen::Library)) {
                m_CurrentScreen = AppScreen::Library;
            }
            if (ImGui::MenuItem("New Page", "Ctrl+N")) {
                if (m_CurrentScreen == AppScreen::DocumentWorkspace) {
                    repo.AddPage("New Page", "GENERAL");
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save All", "Ctrl+S")) {
                repo.Save();
                sm.Save();
            }
            ImGui::Separator();
            auto* curBook = repo.GetActiveBook();
            auto* curPage = repo.GetActivePage();

            if (ImGui::BeginMenu("Export Full Notebook...", curBook != nullptr)) {
                if (ImGui::MenuItem("EPUB E-Book (.epub)")) {
                    std::string out;
                    if (ExportManager::ExportBook(*curBook, ExportFormat::EPUB, out)) {
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
                    if (ExportManager::ExportBook(*curBook, ExportFormat::PDF, out)) {
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
                    if (ExportManager::ExportBook(*curBook, ExportFormat::DOCX, out)) {
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
                    if (ExportManager::ExportBook(*curBook, ExportFormat::Markdown, out)) {
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

            if (ImGui::BeginMenu("Export Current Page...", curBook != nullptr && curPage != nullptr)) {
                if (ImGui::MenuItem("PDF Document (.pdf)")) {
                    std::string out;
                    if (ExportManager::ExportCurrentPage(*curBook, *curPage, ExportFormat::PDF, out)) {
                        m_ExportStatusMsg = "Page exported successfully to PDF";
                        m_LastExportedFile = out;
                    } else {
                        m_ExportStatusMsg = "Failed to export page to PDF.";
                        m_LastExportedFile = "";
                    }
                    m_ShowExportModal = true;
                }
                if (ImGui::MenuItem("EPUB E-Book (.epub)")) {
                    std::string out;
                    if (ExportManager::ExportCurrentPage(*curBook, *curPage, ExportFormat::EPUB, out)) {
                        m_ExportStatusMsg = "Page exported successfully to EPUB";
                        m_LastExportedFile = out;
                    } else {
                        m_ExportStatusMsg = "Failed to export page to EPUB.";
                        m_LastExportedFile = "";
                    }
                    m_ShowExportModal = true;
                }
                if (ImGui::MenuItem("Microsoft Word (.docx)")) {
                    std::string out;
                    if (ExportManager::ExportCurrentPage(*curBook, *curPage, ExportFormat::DOCX, out)) {
                        m_ExportStatusMsg = "Page exported successfully to DOCX";
                        m_LastExportedFile = out;
                    } else {
                        m_ExportStatusMsg = "Failed to export page to DOCX.";
                        m_LastExportedFile = "";
                    }
                    m_ShowExportModal = true;
                }
                if (ImGui::MenuItem("Markdown (.md)")) {
                    std::string out;
                    if (ExportManager::ExportCurrentPage(*curBook, *curPage, ExportFormat::Markdown, out)) {
                        m_ExportStatusMsg = "Page exported successfully to Markdown";
                        m_LastExportedFile = out;
                    } else {
                        m_ExportStatusMsg = "Failed to export page to Markdown.";
                        m_LastExportedFile = "";
                    }
                    m_ShowExportModal = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Open Exports Folder...")) {
                ExportManager::OpenExportFolder(AppPaths::GetBackupsDir());
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Settings & Preferences...", "Ctrl+,")) {
                m_ShowSettingsModal = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                if (m_Window) glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Library View", nullptr, m_CurrentScreen == AppScreen::Library)) {
                m_CurrentScreen = AppScreen::Library;
            }
            if (ImGui::MenuItem("Editor Workspace", nullptr, m_CurrentScreen == AppScreen::DocumentWorkspace)) {
                m_CurrentScreen = AppScreen::DocumentWorkspace;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Theme")) {
            const ThemePreset presets[] = {
                ThemePreset::DarkModern,
                ThemePreset::Paper,
                ThemePreset::DeepPurple,
                ThemePreset::Sapphire,
                ThemePreset::MinimalLight,
                ThemePreset::SepiaWarm
            };
            for (int i = 0; i < 6; ++i) {
                if (ImGui::MenuItem(SettingsManager::GetThemeName(presets[i]), nullptr, s.theme == presets[i])) {
                    s.theme = presets[i];
                    sm.ApplyTheme();
                    sm.Save();
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Check for Updates...")) {
                UpdateManager::Get().CheckForUpdates(true);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Keyboard Shortcuts")) {
                m_ShowShortcutsModal = true;
            }
            if (ImGui::MenuItem("Settings & Preferences...", "Ctrl+,")) {
                m_ShowSettingsModal = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("About Application")) {
                m_ShowAboutModal = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // Pantalla activa: Biblioteca o Espacio de Trabajo
    if (m_CurrentScreen == AppScreen::Library) {
        RenderLibraryView();
    } else {
        RenderWorkspaceView();
    }

    ImGui::End(); // RootHostWindow

    // Modales
    RenderAboutModal();
    RenderShortcutsModal();
    RenderExportModal();
    RenderGlobalUpdatePopup();
    RenderSettingsModal();
}

void UIManager::RenderLibraryView() {
    float availH = ImGui::GetContentRegionAvail().y - 30.0f;
    ImGui::BeginChild("##LibraryContainer", ImVec2(0.0f, availH), false, ImGuiWindowFlags_NoScrollbar);
    m_LibraryPanel->Render();
    ImGui::EndChild();

    RenderStatusBar();
}

void UIManager::RenderWorkspaceView() {
    RenderWorkspaceHeader();

    float totalW = ImGui::GetContentRegionAvail().x;
    float totalH = ImGui::GetContentRegionAvail().y - 28.0f;
    if (totalH < 200.0f) totalH = 200.0f;

    // Proporciones balanceadas de las 3 columnas
    float spacing = 8.0f;
    float leftW = std::clamp(totalW * 0.22f, 220.0f, 320.0f);
    float rightW = std::clamp(totalW * 0.26f, 260.0f, 360.0f);
    float centerW = totalW - leftW - rightW - (spacing * 2.0f);
    if (centerW < 300.0f) centerW = 300.0f;

    // COLUMNA 1: Izquierda (Hojas del Documento)
    RenderSheetsColumn(leftW, totalH);

    ImGui::SameLine(0.0f, spacing);

    // COLUMNA 2: Centro (Editor del Documento)
    ImGui::BeginChild("##CenterEditorColumn", ImVec2(centerW, totalH), true);
    m_EditorPanel->Render();
    ImGui::EndChild();

    ImGui::SameLine(0.0f, spacing);

    // COLUMNA 3: Derecha (Notas, Marcadores e Info)
    ImGui::BeginChild("##RightInfoColumn", ImVec2(rightW, totalH), true);
    m_TocBookmarksPanel->Render();
    ImGui::EndChild();

    RenderStatusBar();
}

void UIManager::RenderWorkspaceHeader() {
    auto& repo = BookRepository::Get();
    auto* book = repo.GetActiveBook();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

    // Botón para volver a la biblioteca
    ImGui::PushStyleColor(ImGuiCol_Button, ToVec4(theme.surface1));
    if (ImGui::Button("Back to Library", ImVec2(160.0f, 32.0f))) {
        m_CurrentScreen = AppScreen::Library;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // Título del Documento
    if (book) {
        ImGui::TextColored(ToVec4(theme.accentLight), "%s", book->title.c_str());
        if (!book->subtitle.empty()) {
            ImGui::SameLine();
            ImGui::TextColored(ToVec4(theme.textDim), "(%s)", book->subtitle.c_str());
        }
    }

    // Navegación secuencial de páginas
    if (book && book->GetTotalPages() > 0) {
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 330.0f);

        bool hasPrev = repo.HasPrevPage();
        if (!hasPrev) ImGui::BeginDisabled();
        if (ImGui::Button("< Previous", ImVec2(85.0f, 30.0f))) {
            repo.PrevPage();
        }
        if (!hasPrev) ImGui::EndDisabled();

        ImGui::SameLine();

        int curGlobal = book->GetGlobalPageIndex(repo.GetActiveChapterIndex(), repo.GetActivePageIndex()) + 1;
        int total = book->GetTotalPages();
        ImGui::TextColored(ToVec4(theme.textDim), "%d / %d", curGlobal, total);

        ImGui::SameLine();

        bool hasNext = repo.HasNextPage();
        if (!hasNext) ImGui::BeginDisabled();
        if (ImGui::Button("Next >", ImVec2(85.0f, 30.0f))) {
            repo.NextPage();
        }
        if (!hasNext) ImGui::EndDisabled();
    }

    ImGui::SameLine(ImGui::GetWindowWidth() - 110.0f);

    // Botón Guardar
    if (ImGui::Button("Save", ImVec2(90.0f, 30.0f))) {
        repo.Save();
        SettingsManager::Get().Save();
    }

    ImGui::PopStyleVar();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void UIManager::RenderSheetsColumn(float width, float height) {
    auto& repo = BookRepository::Get();
    auto* book = repo.GetActiveBook();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    ImGui::BeginChild("##LeftSheetsColumn", ImVec2(width, height), true);

    // Encabezado de la columna
    ImGui::TextColored(ToVec4(theme.accentLight), "NOTEBOOK OUTLINE");
    ImGui::Spacing();

    // Botones de acción rápida
    float halfBtn = (ImGui::GetContentRegionAvail().x - 6.0f) * 0.5f;
    if (ImGui::Button("+ Page", ImVec2(halfBtn, 28.0f))) {
        m_OpenNewSheetModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("+ Section", ImVec2(halfBtn, 28.0f))) {
        m_OpenNewSectionModal = true;
    }

    ImGui::Spacing();

    // Filtro de hojas
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputTextWithHint("##SheetFilter", "Filter pages...", m_SheetFilter, sizeof(m_SheetFilter));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (!book) {
        ImGui::TextColored(ToVec4(theme.textDim), "No active notebook.");
        ImGui::EndChild();
        return;
    }

    int activeCh = repo.GetActiveChapterIndex();
    int activePg = repo.GetActivePageIndex();

    std::string filterStr = m_SheetFilter;
    std::transform(filterStr.begin(), filterStr.end(), filterStr.begin(), ::tolower);

    // Árbol jerárquico de Secciones y Hojas
    for (int c = 0; c < (int)book->chapters.size(); ++c) {
        auto& ch = book->chapters[c];
        ImGui::PushID(c);

        bool isChActive = (c == activeCh);
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
        if (isChActive) flags |= ImGuiTreeNodeFlags_Selected;

        std::string chLabel = ch.title + " (" + std::to_string(ch.pages.size()) + ")";
        bool open = ImGui::TreeNodeEx("##ChNode", flags, "%s", chLabel.c_str());

        if (ImGui::IsItemClicked()) {
            repo.SetActiveChapter(c);
        }

        // Context menu para la sección
        if (ImGui::BeginPopupContextItem("##ChCtx")) {
            if (ImGui::MenuItem("Add Page to this section")) {
                repo.SetActiveChapter(c);
                repo.AddPage("New Page", "GENERAL");
            }
            if (book->chapters.size() > 1 && ImGui::MenuItem("Delete Section")) {
                repo.DeleteChapter(c);
            }
            ImGui::EndPopup();
        }

        if (open) {
            for (int p = 0; p < (int)ch.pages.size(); ++p) {
                const auto& page = ch.pages[p];

                // Filtrado
                if (!filterStr.empty()) {
                    std::string lowerTitle = page.title;
                    std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
                    if (lowerTitle.find(filterStr) == std::string::npos) {
                        continue;
                    }
                }

                ImGui::PushID(p);

                bool isPgActive = (c == activeCh && p == activePg);
                std::string star = page.isBookmarked ? "* " : "  ";
                std::string pgLabel = star + page.title;

                if (ImGui::Selectable(pgLabel.c_str(), isPgActive)) {
                    repo.SetActiveIndices(repo.GetActiveBookIndex(), c, p);
                }

                // Menú contextual para la hoja
                if (ImGui::BeginPopupContextItem("##PageCtx")) {
                    if (ImGui::MenuItem(page.isBookmarked ? "Remove Bookmark" : "Add Bookmark")) {
                        auto* curP = repo.GetActivePage();
                        if (curP) curP->isBookmarked = !curP->isBookmarked;
                        repo.MarkDirty();
                    }
                    if (ImGui::MenuItem("Duplicate Page")) {
                        repo.DuplicateCurrentPage();
                    }
                    if (ch.pages.size() > 1 && ImGui::MenuItem("Delete Page")) {
                        repo.DeletePage(p);
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    // Modal para nueva hoja
    if (m_OpenNewSheetModal) {
        ImGui::OpenPopup("Add New Page");
        m_OpenNewSheetModal = false;
    }
    if (ImGui::BeginPopupModal("Add New Page", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("New page title:");
        ImGui::InputText("Title", m_NewSheetTitle, sizeof(m_NewSheetTitle));
        ImGui::Spacing();
        if (ImGui::Button("Add", ImVec2(100, 28))) {
            repo.AddPage(m_NewSheetTitle, "GENERAL");
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 28))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Modal para nueva sección
    if (m_OpenNewSectionModal) {
        ImGui::OpenPopup("Add New Section");
        m_OpenNewSectionModal = false;
    }
    if (ImGui::BeginPopupModal("Add New Section", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("New section title:");
        ImGui::InputText("Title", m_NewSectionTitle, sizeof(m_NewSectionTitle));
        ImGui::Spacing();
        if (ImGui::Button("Create", ImVec2(100, 28))) {
            repo.AddChapter(m_NewSectionTitle);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 28))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();
}

void UIManager::RenderStatusBar() {
    auto& repo = BookRepository::Get();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    Book* book = repo.GetActiveBook();
    Chapter* ch = repo.GetActiveChapter();
    Page* page = repo.GetActivePage();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ToVec4(theme.surface0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 4));

    ImGui::BeginChild("##StatusBar", ImVec2(ImGui::GetContentRegionAvail().x, 26.0f), false);

    if (m_CurrentScreen == AppScreen::DocumentWorkspace && book && ch && page) {
        int curGlobalPage = book->GetGlobalPageIndex(repo.GetActiveChapterIndex(), repo.GetActivePageIndex()) + 1;
        int totalPages = book->GetTotalPages();

        ImGui::TextColored(ToVec4(theme.accent), "%s", book->title.c_str());
        ImGui::SameLine();
        ImGui::TextColored(ToVec4(theme.textDim), " >  %s", ch->title.c_str());
        ImGui::SameLine();
        ImGui::TextColored(ToVec4(theme.accentLight), " >  %s", page->title.c_str());
        ImGui::SameLine();
        ImGui::TextColored(ToVec4(theme.textDim), " | Page %d of %d", curGlobalPage, totalPages);
        ImGui::SameLine();
        ImGui::TextColored(ToVec4(theme.textFaint), " | %d words", page->GetWordCount());
    } else {
        ImGui::TextColored(ToVec4(theme.textDim), "WitcherNotes ready. Mode: %s",
                           (m_CurrentScreen == AppScreen::Library ? "Library" : "Editor"));
    }

    // Indicador de autoguardado a la derecha
    const char* saveStatus = repo.IsDirty() ? "* Unsaved changes" : "Saved";
    float saveW = ImGui::CalcTextSize(saveStatus).x;
    ImGui::SameLine(ImGui::GetWindowWidth() - saveW - 20.0f);
    ImGui::TextColored(repo.IsDirty() ? ToVec4(theme.accent) : ToVec4(theme.textDim), "%s", saveStatus);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void UIManager::RenderAboutModal() {
    if (m_ShowAboutModal) {
        ImGui::OpenPopup("About Application");
        m_ShowAboutModal = false;
    }

    if (ImGui::BeginPopupModal("About Application", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

        ImGui::TextColored(ToVec4(theme.accentLight), APP_NAME);
        ImGui::TextColored(ToVec4(theme.textDim), APP_DESCRIPTION);
        ImGui::TextColored(ToVec4(theme.textFaint), "Version %s", APP_VERSION_STRING);
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Modern, lightweight desktop notebook built with C++17 and Dear ImGui.");
        ImGui::Text("Engineered for structured knowledge management, reading mode, and Markdown notes.");
        ImGui::Spacing();
        ImGui::TextColored(ToVec4(theme.accent), "100% Free and Open Source (MIT License)");
        ImGui::Spacing();

        if (ImGui::Button("Close", ImVec2(120, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void UIManager::RenderShortcutsModal() {
    if (m_ShowShortcutsModal) {
        ImGui::OpenPopup("Keyboard Shortcuts");
        m_ShowShortcutsModal = false;
    }

    if (ImGui::BeginPopupModal("Keyboard Shortcuts", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

        ImGui::TextColored(ToVec4(theme.accent), "Key Commands:");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Ctrl + S             : Save Changes");
        ImGui::Text("Ctrl + N             : New Page");
        ImGui::Text("Ctrl + B             : Toggle Library / Workspace");
        ImGui::Text("Left Arrow / A       : Previous Page");
        ImGui::Text("Right Arrow / D      : Next Page");
        ImGui::Spacing();

        if (ImGui::Button("Got It", ImVec2(120, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void UIManager::RenderExportModal() {
    if (m_ShowExportModal) {
        ImGui::OpenPopup("Export Result##UIMenuModal");
        m_ShowExportModal = false;
    }

    if (ImGui::BeginPopupModal("Export Result##UIMenuModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

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

void UIManager::RenderGlobalUpdatePopup() {
    auto& updater = UpdateManager::Get();
    if (updater.ShouldShowModal()) {
        ImGui::OpenPopup("Update Available##GlobalModal");
        updater.SetShowModal(false);
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 0));

    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    ImGui::PushStyleColor(ImGuiCol_PopupBg, ToVec4(theme.base));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 20.0f));

    if (ImGui::BeginPopupModal("Update Available##GlobalModal", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {

        ImGui::TextColored(ToVec4(theme.textPrimary), "New version available");
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.96f, 0.65f, 0.14f, 1.0f), "v%s", updater.GetLatestVersion().c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (updater.IsDownloading()) {
            float pct   = updater.GetDownloadProgress();
            float dlMB  = updater.GetDownloadedMB();
            float totMB = updater.GetTotalMB();
            float speed = updater.GetDownloadSpeedMBs();

            ImGui::TextColored(ToVec4(theme.accentLight), "Downloading update...");
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ToVec4(theme.accent));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ToVec4(theme.surface0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::ProgressBar(pct, ImVec2(-1.0f, 12.0f), "");
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);

            ImGui::Spacing();

            if (totMB > 0.0f) {
                ImGui::TextColored(ToVec4(theme.textDim), "%.1f MB / %.1f MB   %.1f MB/s   %d%%",
                                   dlMB, totMB, speed, (int)(pct * 100.0f));
            } else {
                ImGui::TextColored(ToVec4(theme.textDim), "Downloading...  %d%%", (int)(pct * 100.0f));
            }

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.85f, 0.35f, 0.35f, 1.0f), "Do not close the application during download.");
        } else {
            ImGui::TextColored(ToVec4(theme.textPrimary),
                "A new version is available to download and install.\n"
                "Continuing will download the update package and apply it.");

            if (!updater.GetChangelog().empty()) {
                ImGui::Spacing();
                ImGui::TextColored(ToVec4(theme.accentLight), "Changelog:");
                ImGui::BeginChild("##ChangelogBox", ImVec2(-1.0f, 90.0f), true);
                ImGui::TextWrapped("%s", updater.GetChangelog().c_str());
                ImGui::EndChild();
            }

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.0f, 7.0f));

            if (ImGui::Button("Download & Install", ImVec2(180.0f, 34.0f))) {
                updater.DownloadAndInstall();
            }

            ImGui::SameLine(0, 10.0f);

            if (ImGui::Button("Later", ImVec2(110.0f, 34.0f))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::PopStyleVar(2);
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void UIManager::RenderSettingsModal() {
    if (m_ShowSettingsModal) {
        ImGui::OpenPopup("Settings & Preferences##GlobalModal");
        m_ShowSettingsModal = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(720.0f, 580.0f), ImGuiCond_Appearing);

    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ToVec4(theme.base));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));

    if (ImGui::BeginPopupModal("Settings & Preferences##GlobalModal", nullptr, ImGuiWindowFlags_None)) {
        float availH = ImGui::GetContentRegionAvail().y - 42.0f;
        ImGui::BeginChild("##SettingsScrollArea", ImVec2(0.0f, availH), false);
        m_SettingsPanel->Render();
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Close", ImVec2(120.0f, 30.0f))) {
            SettingsManager::Get().Save();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

} // namespace WitcherNotes::UI

