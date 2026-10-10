#pragma once

#include <imgui.h>
#include <string>
#include <functional>

namespace WitcherNotes::UI {

class LibraryPanel {
public:
    LibraryPanel();
    void Render();

    void SetOnOpenDocument(std::function<void(int)> callback) {
        m_OnOpenDocument = callback;
    }

private:
    void RenderTopBar();
    void RenderDocumentGrid();
    void RenderSearchResults();
    void RenderNewDocumentModal();
    void RenderRenameDocumentModal();
    void RenderExportModal();

    std::function<void(int)> m_OnOpenDocument;

    char m_SearchQuery[128] = {};
    char m_NewDocTitle[128] = "Nuevo Cuaderno";
    char m_NewDocSubtitle[128] = "Notas personales y proyectos";
    char m_NewDocAuthor[128] = "Usuario";

    bool m_OpenNewDocModal = false;
    bool m_OpenRenameDocModal = false;
    int m_DocToRename = -1;
    char m_RenameDocTitle[128] = {};

    bool m_ShowExportModal = false;
    std::string m_ExportStatusMsg;
    std::string m_LastExportedFile;
};

} // namespace WitcherNotes::UI
