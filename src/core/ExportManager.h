#pragma once

#include <string>
#include "BookModel.h"

namespace WitcherNotes::Core {

enum class ExportFormat {
    EPUB = 0,
    PDF,
    DOCX,
    Markdown
};

class ExportManager {
public:
    static bool ExportBook(const Book& book, ExportFormat format, std::string& outFilePath);
    static bool ExportCurrentPage(const Book& book, const Page& page, ExportFormat format, std::string& outFilePath);
    static void OpenExportFolder(const std::string& filePath);
    static const char* GetFormatExtension(ExportFormat format);
    static const char* GetFormatName(ExportFormat format);
};

} // namespace WitcherNotes::Core

