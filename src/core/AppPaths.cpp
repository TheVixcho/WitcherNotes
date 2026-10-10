#include "AppPaths.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#include <climits>
#endif

namespace WitcherNotes::Core {

std::string AppPaths::GetAppDataDir() {
#ifdef _WIN32
    const char* appData = std::getenv("APPDATA");
    if (!appData) return "NotesAppData";
    std::filesystem::path p = std::filesystem::path(appData) / "WitcherNotes";
#else
    const char* home = std::getenv("HOME");
    if (!home) return "NotesAppData";
    std::filesystem::path p = std::filesystem::path(home) / ".config" / "WitcherNotes";
#endif
    return p.string();
}

std::string AppPaths::GetConfigFilePath() {
    return (std::filesystem::path(GetAppDataDir()) / "settings.json").string();
}

std::string AppPaths::GetBooksFilePath() {
    return (std::filesystem::path(GetAppDataDir()) / "documents.json").string();
}

std::string AppPaths::GetBackupsDir() {
    return (std::filesystem::path(GetAppDataDir()) / "backups").string();
}

bool AppPaths::EnsureDirectories() {
    try {
        std::filesystem::create_directories(GetAppDataDir());
        std::filesystem::create_directories(GetBackupsDir());
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Notes] Error creando directorios: " << e.what() << "\n";
        return false;
    }
}

std::string AppPaths::ResolveAssetPath(const std::string& path) {
    if (std::filesystem::exists(path)) {
        return path;
    }

    // Probar varios posibles prefijos
    static const std::vector<std::string> prefixes = {
        "",
        "assets/",
        "bin/assets/",
        "../assets/",
        "../../assets/",
        "assets/fonts/",
        "assets/images/",
        "bin/assets/fonts/",
        "bin/assets/images/"
    };

    for (const auto& pref : prefixes) {
        std::filesystem::path candidate = std::filesystem::path(pref) / path;
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    return path;
}

void AppPaths::ChangeToExecutableDirectory() {
#ifdef _WIN32
    // Windows
#else
    char exePath[PATH_MAX] = {};
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len > 0) {
        exePath[len] = '\0';
        std::filesystem::path dir = std::filesystem::path(exePath).parent_path();
        chdir(dir.c_str());
    }
#endif
}

} // namespace WitcherNotes::Core
