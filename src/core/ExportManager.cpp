#include "ExportManager.h"
#include "AppPaths.h"
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include <ctime>

using json = nlohmann::json;

namespace WitcherNotes::Core {

static std::string SanitizeFilename(const std::string& name) {
    std::string safe = name;
    for (char& c : safe) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }
    return safe.empty() ? "Documento" : safe;
}

const char* ExportManager::GetFormatExtension(ExportFormat format) {
    switch (format) {
        case ExportFormat::EPUB:     return ".epub";
        case ExportFormat::PDF:      return ".pdf";
        case ExportFormat::DOCX:     return ".docx";
        case ExportFormat::Markdown: return ".md";
        default:                     return ".txt";
    }
}

const char* ExportManager::GetFormatName(ExportFormat format) {
    switch (format) {
        case ExportFormat::EPUB:     return "Libro Digital EPUB (.epub)";
        case ExportFormat::PDF:      return "Documento PDF (.pdf)";
        case ExportFormat::DOCX:     return "Microsoft Word DOCX (.docx)";
        case ExportFormat::Markdown: return "Markdown (.md)";
        default:                     return "Texto Plano";
    }
}

static std::string FindExporterScript() {
    std::vector<std::string> candidates = {
        "assets/scripts/exporter.py",
        "scripts/exporter.py",
        "../scripts/exporter.py",
        "../../scripts/exporter.py",
        "bin/assets/scripts/exporter.py"
    };

    for (const auto& c : candidates) {
        std::string resolved = AppPaths::ResolveAssetPath(c);
        if (std::filesystem::exists(resolved)) {
            return std::filesystem::canonical(resolved).string();
        }
    }
    return "assets/scripts/exporter.py";
}

bool ExportManager::ExportBook(const Book& book, ExportFormat format, std::string& outFilePath) {
    std::string exportDir = AppPaths::GetBackupsDir();
    std::filesystem::create_directories(exportDir);

    std::string ext = GetFormatExtension(format);
    std::string safeTitle = SanitizeFilename(book.title);
    outFilePath = exportDir + "/" + safeTitle + ext;

    // Serializar a JSON temporal
    json jb;
    jb["title"] = book.title;
    jb["subtitle"] = book.subtitle;
    jb["author"] = book.author;
    jb["category"] = book.category;
    jb["updatedAt"] = book.updatedAt;

    json jChapters = json::array();
    for (const auto& ch : book.chapters) {
        json jch;
        jch["title"] = ch.title;
        jch["description"] = ch.description;

        json jPages = json::array();
        for (const auto& pg : ch.pages) {
            json jpg;
            jpg["title"] = pg.title;
            jpg["content"] = pg.content;
            jpg["marginalia"] = pg.marginalia;
            jpg["signTag"] = pg.signTag;
            jpg["updatedAt"] = pg.updatedAt;
            jPages.push_back(jpg);
        }
        jch["pages"] = jPages;
        jChapters.push_back(jch);
    }
    jb["chapters"] = jChapters;

    std::string tempJsonPath = AppPaths::GetAppDataDir() + "/temp_export.json";
    {
        std::ofstream out(tempJsonPath);
        if (!out.is_open()) return false;
        out << jb.dump(4);
    }

    std::string scriptPath = FindExporterScript();
    std::string fmtStr = (format == ExportFormat::EPUB) ? "epub" :
                         (format == ExportFormat::PDF)  ? "pdf"  :
                         (format == ExportFormat::DOCX) ? "docx" : "md";

    // Intentar ejecutar directamente con python3, y con fallback a flatpak-spawn
    std::string cmd = "python3 \"" + scriptPath + "\" \"" + tempJsonPath + "\" \"" + outFilePath + "\" " + fmtStr;
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::string hostCmd = "flatpak-spawn --host python3 \"" + scriptPath + "\" \"" + tempJsonPath + "\" \"" + outFilePath + "\" " + fmtStr;
        ret = std::system(hostCmd.c_str());
    }

    // Limpiar archivo temporal
    std::error_code ec;
    std::filesystem::remove(tempJsonPath, ec);

    return (ret == 0 && std::filesystem::exists(outFilePath));
}

bool ExportManager::ExportCurrentPage(const Book& book, const Page& page, ExportFormat format, std::string& outFilePath) {
    std::string exportDir = AppPaths::GetBackupsDir();
    std::filesystem::create_directories(exportDir);

    std::string ext = GetFormatExtension(format);
    std::string safeTitle = SanitizeFilename(book.title + "_" + page.title);
    outFilePath = exportDir + "/" + safeTitle + ext;

    json jb;
    jb["title"] = page.title;
    jb["subtitle"] = "De: " + book.title;
    jb["author"] = book.author;
    jb["category"] = page.signTag;
    jb["updatedAt"] = page.updatedAt;

    json jChapters = json::array();
    json jch;
    jch["title"] = page.title;
    jch["description"] = "";

    json jPages = json::array();
    json jpg;
    jpg["title"] = page.title;
    jpg["content"] = page.content;
    jpg["marginalia"] = page.marginalia;
    jpg["signTag"] = page.signTag;
    jpg["updatedAt"] = page.updatedAt;
    jPages.push_back(jpg);

    jch["pages"] = jPages;
    jChapters.push_back(jch);
    jb["chapters"] = jChapters;

    std::string tempJsonPath = AppPaths::GetAppDataDir() + "/temp_export.json";
    {
        std::ofstream out(tempJsonPath);
        if (!out.is_open()) return false;
        out << jb.dump(4);
    }

    std::string scriptPath = FindExporterScript();
    std::string fmtStr = (format == ExportFormat::EPUB) ? "epub" :
                         (format == ExportFormat::PDF)  ? "pdf"  :
                         (format == ExportFormat::DOCX) ? "docx" : "md";

    std::string cmd = "python3 \"" + scriptPath + "\" \"" + tempJsonPath + "\" \"" + outFilePath + "\" " + fmtStr;
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::string hostCmd = "flatpak-spawn --host python3 \"" + scriptPath + "\" \"" + tempJsonPath + "\" \"" + outFilePath + "\" " + fmtStr;
        ret = std::system(hostCmd.c_str());
    }

    std::error_code ec;
    std::filesystem::remove(tempJsonPath, ec);

    return (ret == 0 && std::filesystem::exists(outFilePath));
}

void ExportManager::OpenExportFolder(const std::string& filePath) {
    std::filesystem::path p(filePath);
    std::string dir = p.parent_path().string();
    if (dir.empty()) dir = AppPaths::GetBackupsDir();

    std::string cmd = "xdg-open \"" + dir + "\" 2>/dev/null || flatpak-spawn --host xdg-open \"" + dir + "\" 2>/dev/null";
    std::system(cmd.c_str());
}

} // namespace WitcherNotes::Core

