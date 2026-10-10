#pragma once

#include <string>

namespace WitcherNotes::Core {

class AppPaths {
public:
    static std::string GetAppDataDir();
    static std::string GetConfigFilePath();
    static std::string GetBooksFilePath();
    static std::string GetBackupsDir();
    static std::string ResolveAssetPath(const std::string& relativeOrExactPath);
    static bool EnsureDirectories();
    static void ChangeToExecutableDirectory();
};

} // namespace WitcherNotes::Core
