#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>
#include <algorithm>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Version.h"
#include "core/AppPaths.h"
#include "core/SettingsManager.h"
#include "core/BookModel.h"
#include "ui/UIManager.h"

namespace {
constexpr int kMainWBase = 1380;
constexpr int kMainHBase = 820;
}

static float DetectDpiScale() {
    float scale = 1.0f;
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (primary) {
        float sx = 1.0f, sy = 1.0f;
        glfwGetMonitorContentScale(primary, &sx, &sy);
        if (sx > 0.0f) scale = sx;
    }
    return scale;
}

static GLFWwindow* CreateMainWindow(int mainW, int mainH) {
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE,   GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(mainW, mainH, APP_NAME " - " APP_DESCRIPTION, nullptr, nullptr);
    if (!window) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        window = glfwCreateWindow(mainW, mainH, APP_NAME " - " APP_DESCRIPTION, nullptr, nullptr);
    }
    if (!window) return nullptr;

    // Centrar la ventana en el monitor principal
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (primary) {
        const GLFWvidmode* vm = glfwGetVideoMode(primary);
        if (vm) {
            int mx = 0, my = 0;
            glfwGetMonitorPos(primary, &mx, &my);
            glfwSetWindowPos(window, mx + (vm->width - mainW) / 2, my + (vm->height - mainH) / 2);
        }
    }

    // Cargar icono si existe
    std::string iconPath = WitcherNotes::Core::AppPaths::ResolveAssetPath("witchernotes.png");
    if (std::filesystem::exists(iconPath)) {
        GLFWimage images[1];
        images[0].pixels = stbi_load(iconPath.c_str(), &images[0].width, &images[0].height, 0, 4);
        if (images[0].pixels) {
            glfwSetWindowIcon(window, 1, images);
            stbi_image_free(images[0].pixels);
        }
    }

    return window;
}

static void LoadMainFonts(ImGuiIO& io, float dpiScale) {
    static ImVector<ImWchar> s_Ranges;
    if (s_Ranges.empty()) {
        ImFontGlyphRangesBuilder b;
        b.AddRanges(io.Fonts->GetGlyphRangesDefault());
        b.AddRanges(io.Fonts->GetGlyphRangesGreek());
        b.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
        static const ImWchar kExtraSymbols[] = {
            0x0100, 0x024F, // Latin Extended
            0x2000, 0x206F, // General Punctuation
            0x2190, 0x21FF, // Arrows
            0x25A0, 0x25FF, // Geometric Shapes
            0x2600, 0x26FF, // Miscellaneous Symbols (⚙, ⚠, ⚡)
            0x2700, 0x27BF, // Dingbats (✔, ✦, ✨, ✪)
            0
        };
        b.AddRanges(kExtraSymbols);
        b.BuildRanges(&s_Ranges);
    }

    std::string mainFontPath = WitcherNotes::Core::AppPaths::ResolveAssetPath("assets/fonts/OpenSans-Regular.ttf");
    if (!std::filesystem::exists(mainFontPath)) {
        mainFontPath = WitcherNotes::Core::AppPaths::ResolveAssetPath("OpenSans-Regular.ttf");
    }

    ImFont* mainFont = nullptr;
    if (std::filesystem::exists(mainFontPath)) {
        ImFontConfig cfg;
        cfg.OversampleH = 2;
        cfg.OversampleV = 2;
        mainFont = io.Fonts->AddFontFromFileTTF(mainFontPath.c_str(), 16.0f * dpiScale, &cfg, s_Ranges.Data);
    }

    std::string symbolFont = WitcherNotes::Core::AppPaths::ResolveAssetPath("assets/fonts/Symbola.ttf");
    if (std::filesystem::exists(symbolFont)) {
        ImFontConfig mergeCfg;
        mergeCfg.MergeMode = true;
        mergeCfg.OversampleH = 1;
        io.Fonts->AddFontFromFileTTF(symbolFont.c_str(), 15.0f * dpiScale, &mergeCfg, s_Ranges.Data);
    }

    if (!mainFont) {
        io.Fonts->AddFontDefault();
    }
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    WitcherNotes::Core::AppPaths::ChangeToExecutableDirectory();
    WitcherNotes::Core::AppPaths::EnsureDirectories();

    std::cout << "[Notes] Iniciando Cuaderno de Notas...\n";

    if (!glfwInit()) {
        std::cerr << "[Notes] Falló glfwInit()\n";
        return -1;
    }

    float dpiScale = DetectDpiScale();
    int mainW = (int)(kMainWBase * dpiScale);
    int mainH = (int)(kMainHBase * dpiScale);

    GLFWwindow* mainWindow = CreateMainWindow(mainW, mainH);
    if (!mainWindow) {
        std::cerr << "[Notes] Error crítico: no se pudo crear la ventana principal.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(mainWindow);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK && glewStatus != GLEW_ERROR_NO_GLX_DISPLAY) {
        std::cerr << "[Notes] Advertencia en glewInit: " << glewGetErrorString(glewStatus) << "\n";
    }

    // Inicializar ImGui
    IMGUI_CHECKVERSION();
    ImGuiContext* mainContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(mainContext);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = "notes_ui.ini";

    LoadMainFonts(io, dpiScale);

    ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    auto& sm = WitcherNotes::Settings::SettingsManager::Get();
    sm.ApplyTheme();
    ImGui::GetStyle().ScaleAllSizes(dpiScale);

    // Cargar documentos
    WitcherNotes::Core::BookRepository::Get().Load();

    WitcherNotes::UI::UIManager uiManager;
    uiManager.Initialize(mainWindow);

    // Mostrar ventana
    glfwShowWindow(mainWindow);
    glfwFocusWindow(mainWindow);

    std::cout << "[Notes] Aplicación lista. Abriendo en vista Biblioteca...\n";

    // Bucle Principal
    while (!glfwWindowShouldClose(mainWindow)) {
        glfwPollEvents();

        int displayW, displayH;
        glfwGetFramebufferSize(mainWindow, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);

        auto curTheme = sm.GetCurrentThemeColors();
        glClearColor(curTheme.base[0], curTheme.base[1], curTheme.base[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        uiManager.RenderAll();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(mainWindow);
    }

    std::cout << "[Notes] Guardando y cerrando...\n";
    uiManager.Shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(mainContext);

    glfwDestroyWindow(mainWindow);
    glfwTerminate();

    return 0;
}
