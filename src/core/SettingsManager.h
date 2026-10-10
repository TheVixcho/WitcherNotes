#pragma once

#include <string>
#include <imgui.h>

namespace WitcherNotes::Settings {

enum class ThemePreset {
    DarkModern = 0,     // Oscuro Moderno (Grafito y Ámbar)
    Paper,              // Papel Clásico (Cálido y Tinta Nogal)
    DeepPurple,         // Púrpura Elegante / Obsidiana
    Sapphire,           // Azul Zafiro / Medianoche
    MinimalLight,       // Claro Minimalista / Alto Contraste
    SepiaWarm,          // Sepia Suave

    // Aliases para retrocompatibilidad
    GeraltDark = DarkModern,
    Parchment = Paper,
    YenneferLilac = DeepPurple,
    OxenfurtSapphire = Sapphire,
    EInkReader = MinimalLight
};

enum class ReaderMode {
    TwoPageSpread = 0,  // Dos Páginas (Libro Abierto)
    SinglePage,         // Una Página centrada (E-Reader clásico)
    ContinuousScroll    // Pergamino continuo
};

enum class WorkspaceView {
    ReaderOnly = 0,
    EditorOnly,
    SplitReaderEditor,
    FullWorkspace
};

struct ThemeColors {
    float base[4];
    float surface0[4];
    float surface1[4];
    float surface2[4];
    float surface3[4];

    float accent[4];
    float accentLight[4];
    float accentDim[4];

    float pageBg[4];
    float pageText[4];
    float pageBorder[4];
    float spineShadow[4];

    float textPrimary[4];
    float textDim[4];
    float textFaint[4];

    float border[4];
    float borderFaint[4];
};

struct AppSettings {
    ThemePreset theme = ThemePreset::DarkModern;
    ReaderMode readerMode = ReaderMode::TwoPageSpread;
    WorkspaceView workspaceView = WorkspaceView::SplitReaderEditor;

    float readerFontSize = 18.0f;
    float readerLineSpacing = 1.45f;
    float readerPageMargin = 36.0f;
    int readerFontIndex = 0;

    bool showDropCaps = true;
    bool showPageBorders = true;
    bool showSpineShadow = true;
    bool showMarginalia = true;
    bool showBookmarksRibbon = true;

    bool autoSave = true;
    int autoSaveIntervalSec = 60;

    bool startMaximized = true;
    bool showLibraryPanel = true;
    bool showTocPanel = true;
    bool showSettingsPanel = false;
    bool isFullscreen = false;

    // Actualizaciones automáticas (GitHub)
    bool checkUpdatesOnStartup = true;
    std::string updateChannel = "beta"; // "stable" o "beta"
    std::string githubRepo = "TheVixcho/WitcherNotes";
    std::string githubToken = ""; // Personal Access Token (opcional para repos privados o rate limits)
    std::string lastUpdateCheck = "";
};

class SettingsManager {
public:
    static SettingsManager& Get();

    void Load();
    void Save();

    AppSettings& GetSettings() { return m_Settings; }
    const AppSettings& GetSettings() const { return m_Settings; }

    ThemeColors GetThemeColors(ThemePreset preset) const;
    ThemeColors GetCurrentThemeColors() const { return GetThemeColors(m_Settings.theme); }

    void ApplyTheme();

    static const char* GetThemeName(ThemePreset preset);
    static const char* GetReaderModeName(ReaderMode mode);

private:
    SettingsManager();
    AppSettings m_Settings;
};

} // namespace WitcherNotes::Settings
