#include "SettingsManager.h"
#include "AppPaths.h"
#include <json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;
using namespace WitcherNotes::Core;

namespace WitcherNotes::Settings {

SettingsManager::SettingsManager() {
    Load();
}

SettingsManager& SettingsManager::Get() {
    static SettingsManager instance;
    return instance;
}

const char* SettingsManager::GetThemeName(ThemePreset preset) {
    switch (preset) {
        case ThemePreset::DarkModern:    return "Oscuro Moderno";
        case ThemePreset::Paper:         return "Papel Clásico";
        case ThemePreset::DeepPurple:    return "Púrpura Elegante";
        case ThemePreset::Sapphire:      return "Azul Zafiro";
        case ThemePreset::MinimalLight:  return "Claro Minimalista";
        case ThemePreset::SepiaWarm:     return "Sepia Cálido";
        default:                         return "Por Defecto";
    }
}

const char* SettingsManager::GetReaderModeName(ReaderMode mode) {
    switch (mode) {
        case ReaderMode::TwoPageSpread:    return "Doble Página (Libro Abierto)";
        case ReaderMode::SinglePage:       return "Página Única (Centrado)";
        case ReaderMode::ContinuousScroll: return "Desplazamiento Continuo";
        default:                           return "Estándar";
    }
}

ThemeColors SettingsManager::GetThemeColors(ThemePreset preset) const {
    ThemeColors c{};

    switch (preset) {
        case ThemePreset::DarkModern: {
            // Oscuro Moderno (Grafito y Ámbar)
            c.base[0] = 0.08f; c.base[1] = 0.09f; c.base[2] = 0.10f; c.base[3] = 1.0f;
            c.surface0[0] = 0.11f; c.surface0[1] = 0.12f; c.surface0[2] = 0.14f; c.surface0[3] = 1.0f;
            c.surface1[0] = 0.14f; c.surface1[1] = 0.16f; c.surface1[2] = 0.18f; c.surface1[3] = 1.0f;
            c.surface2[0] = 0.18f; c.surface2[1] = 0.20f; c.surface2[2] = 0.23f; c.surface2[3] = 1.0f;
            c.surface3[0] = 0.23f; c.surface3[1] = 0.25f; c.surface3[2] = 0.29f; c.surface3[3] = 1.0f;

            c.accent[0] = 0.83f; c.accent[1] = 0.64f; c.accent[2] = 0.26f; c.accent[3] = 1.0f; // Ámbar dorado
            c.accentLight[0] = 0.95f; c.accentLight[1] = 0.77f; c.accentLight[2] = 0.42f; c.accentLight[3] = 1.0f;
            c.accentDim[0] = 0.55f; c.accentDim[1] = 0.40f; c.accentDim[2] = 0.15f; c.accentDim[3] = 1.0f;

            c.pageBg[0] = 0.12f; c.pageBg[1] = 0.13f; c.pageBg[2] = 0.15f; c.pageBg[3] = 1.0f;
            c.pageText[0] = 0.90f; c.pageText[1] = 0.91f; c.pageText[2] = 0.93f; c.pageText[3] = 1.0f;
            c.pageBorder[0] = 0.45f; c.pageBorder[1] = 0.35f; c.pageBorder[2] = 0.18f; c.pageBorder[3] = 0.45f;
            c.spineShadow[0] = 0.02f; c.spineShadow[1] = 0.02f; c.spineShadow[2] = 0.03f; c.spineShadow[3] = 0.65f;

            c.textPrimary[0] = 0.92f; c.textPrimary[1] = 0.93f; c.textPrimary[2] = 0.95f; c.textPrimary[3] = 1.0f;
            c.textDim[0] = 0.70f; c.textDim[1] = 0.72f; c.textDim[2] = 0.75f; c.textDim[3] = 1.0f;
            c.textFaint[0] = 0.48f; c.textFaint[1] = 0.50f; c.textFaint[2] = 0.53f; c.textFaint[3] = 1.0f;

            c.border[0] = 0.28f; c.border[1] = 0.26f; c.border[2] = 0.22f; c.border[3] = 0.40f;
            c.borderFaint[0] = 0.20f; c.borderFaint[1] = 0.20f; c.borderFaint[2] = 0.20f; c.borderFaint[3] = 0.25f;
            break;
        }

        case ThemePreset::Paper: {
            // Papel Clásico (Cálido y Elegante)
            c.base[0] = 0.14f; c.base[1] = 0.10f; c.base[2] = 0.07f; c.base[3] = 1.0f;
            c.surface0[0] = 0.18f; c.surface0[1] = 0.13f; c.surface0[2] = 0.09f; c.surface0[3] = 1.0f;
            c.surface1[0] = 0.23f; c.surface1[1] = 0.17f; c.surface1[2] = 0.12f; c.surface1[3] = 1.0f;
            c.surface2[0] = 0.29f; c.surface2[1] = 0.22f; c.surface2[2] = 0.16f; c.surface2[3] = 1.0f;
            c.surface3[0] = 0.35f; c.surface3[1] = 0.27f; c.surface3[2] = 0.20f; c.surface3[3] = 1.0f;

            c.accent[0] = 0.72f; c.accent[1] = 0.24f; c.accent[2] = 0.18f; c.accent[3] = 1.0f; // Carmesí cálido
            c.accentLight[0] = 0.85f; c.accentLight[1] = 0.38f; c.accentLight[2] = 0.28f; c.accentLight[3] = 1.0f;
            c.accentDim[0] = 0.50f; c.accentDim[1] = 0.15f; c.accentDim[2] = 0.10f; c.accentDim[3] = 1.0f;

            c.pageBg[0] = 0.94f; c.pageBg[1] = 0.90f; c.pageBg[2] = 0.78f; c.pageBg[3] = 1.0f;
            c.pageText[0] = 0.17f; c.pageText[1] = 0.12f; c.pageText[2] = 0.08f; c.pageText[3] = 1.0f;
            c.pageBorder[0] = 0.58f; c.pageBorder[1] = 0.44f; c.pageBorder[2] = 0.28f; c.pageBorder[3] = 0.75f;
            c.spineShadow[0] = 0.15f; c.spineShadow[1] = 0.10f; c.spineShadow[2] = 0.05f; c.spineShadow[3] = 0.55f;

            c.textPrimary[0] = 0.93f; c.textPrimary[1] = 0.88f; c.textPrimary[2] = 0.78f; c.textPrimary[3] = 1.0f;
            c.textDim[0] = 0.75f; c.textDim[1] = 0.68f; c.textDim[2] = 0.58f; c.textDim[3] = 1.0f;
            c.textFaint[0] = 0.52f; c.textFaint[1] = 0.46f; c.textFaint[2] = 0.38f; c.textFaint[3] = 1.0f;

            c.border[0] = 0.42f; c.border[1] = 0.32f; c.border[2] = 0.22f; c.border[3] = 0.50f;
            c.borderFaint[0] = 0.30f; c.borderFaint[1] = 0.22f; c.borderFaint[2] = 0.15f; c.borderFaint[3] = 0.30f;
            break;
        }

        case ThemePreset::DeepPurple: {
            // Púrpura Elegante
            c.base[0] = 0.07f; c.base[1] = 0.06f; c.base[2] = 0.09f; c.base[3] = 1.0f;
            c.surface0[0] = 0.10f; c.surface0[1] = 0.08f; c.surface0[2] = 0.14f; c.surface0[3] = 1.0f;
            c.surface1[0] = 0.14f; c.surface1[1] = 0.11f; c.surface1[2] = 0.19f; c.surface1[3] = 1.0f;
            c.surface2[0] = 0.19f; c.surface2[1] = 0.15f; c.surface2[2] = 0.25f; c.surface2[3] = 1.0f;
            c.surface3[0] = 0.25f; c.surface3[1] = 0.20f; c.surface3[2] = 0.32f; c.surface3[3] = 1.0f;

            c.accent[0] = 0.65f; c.accent[1] = 0.42f; c.accent[2] = 0.85f; c.accent[3] = 1.0f;
            c.accentLight[0] = 0.78f; c.accentLight[1] = 0.58f; c.accentLight[2] = 0.95f; c.accentLight[3] = 1.0f;
            c.accentDim[0] = 0.42f; c.accentDim[1] = 0.25f; c.accentDim[2] = 0.58f; c.accentDim[3] = 1.0f;

            c.pageBg[0] = 0.11f; c.pageBg[1] = 0.09f; c.pageBg[2] = 0.15f; c.pageBg[3] = 1.0f;
            c.pageText[0] = 0.94f; c.pageText[1] = 0.90f; c.pageText[2] = 0.98f; c.pageText[3] = 1.0f;
            c.pageBorder[0] = 0.52f; c.pageBorder[1] = 0.32f; c.pageBorder[2] = 0.68f; c.pageBorder[3] = 0.45f;
            c.spineShadow[0] = 0.02f; c.spineShadow[1] = 0.01f; c.spineShadow[2] = 0.04f; c.spineShadow[3] = 0.70f;

            c.textPrimary[0] = 0.94f; c.textPrimary[1] = 0.91f; c.textPrimary[2] = 0.97f; c.textPrimary[3] = 1.0f;
            c.textDim[0] = 0.72f; c.textDim[1] = 0.68f; c.textDim[2] = 0.78f; c.textDim[3] = 1.0f;
            c.textFaint[0] = 0.50f; c.textFaint[1] = 0.46f; c.textFaint[2] = 0.55f; c.textFaint[3] = 1.0f;

            c.border[0] = 0.35f; c.border[1] = 0.22f; c.border[2] = 0.45f; c.border[3] = 0.40f;
            c.borderFaint[0] = 0.22f; c.borderFaint[1] = 0.15f; c.borderFaint[2] = 0.28f; c.borderFaint[3] = 0.25f;
            break;
        }

        case ThemePreset::Sapphire: {
            // Azul Zafiro (Medianoche)
            c.base[0] = 0.06f; c.base[1] = 0.08f; c.base[2] = 0.14f; c.base[3] = 1.0f;
            c.surface0[0] = 0.08f; c.surface0[1] = 0.11f; c.surface0[2] = 0.19f; c.surface0[3] = 1.0f;
            c.surface1[0] = 0.11f; c.surface1[1] = 0.15f; c.surface1[2] = 0.24f; c.surface1[3] = 1.0f;
            c.surface2[0] = 0.15f; c.surface2[1] = 0.20f; c.surface2[2] = 0.31f; c.surface2[3] = 1.0f;
            c.surface3[0] = 0.20f; c.surface3[1] = 0.26f; c.surface3[2] = 0.38f; c.surface3[3] = 1.0f;

            c.accent[0] = 0.28f; c.accent[1] = 0.55f; c.accent[2] = 0.95f; c.accent[3] = 1.0f;
            c.accentLight[0] = 0.48f; c.accentLight[1] = 0.70f; c.accentLight[2] = 1.00f; c.accentLight[3] = 1.0f;
            c.accentDim[0] = 0.18f; c.accentDim[1] = 0.36f; c.accentDim[2] = 0.65f; c.accentDim[3] = 1.0f;

            c.pageBg[0] = 0.09f; c.pageBg[1] = 0.12f; c.pageBg[2] = 0.20f; c.pageBg[3] = 1.0f;
            c.pageText[0] = 0.90f; c.pageText[1] = 0.93f; c.pageText[2] = 0.98f; c.pageText[3] = 1.0f;
            c.pageBorder[0] = 0.24f; c.pageBorder[1] = 0.42f; c.pageBorder[2] = 0.68f; c.pageBorder[3] = 0.50f;
            c.spineShadow[0] = 0.01f; c.spineShadow[1] = 0.02f; c.spineShadow[2] = 0.05f; c.spineShadow[3] = 0.65f;

            c.textPrimary[0] = 0.91f; c.textPrimary[1] = 0.94f; c.textPrimary[2] = 0.98f; c.textPrimary[3] = 1.0f;
            c.textDim[0] = 0.68f; c.textDim[1] = 0.74f; c.textDim[2] = 0.82f; c.textDim[3] = 1.0f;
            c.textFaint[0] = 0.45f; c.textFaint[1] = 0.50f; c.textFaint[2] = 0.60f; c.textFaint[3] = 1.0f;

            c.border[0] = 0.20f; c.border[1] = 0.30f; c.border[2] = 0.48f; c.border[3] = 0.40f;
            c.borderFaint[0] = 0.14f; c.borderFaint[1] = 0.20f; c.borderFaint[2] = 0.32f; c.borderFaint[3] = 0.25f;
            break;
        }

        case ThemePreset::MinimalLight: {
            // Claro Minimalista (Alto Contraste)
            c.base[0] = 0.95f; c.base[1] = 0.95f; c.base[2] = 0.96f; c.base[3] = 1.0f;
            c.surface0[0] = 0.90f; c.surface0[1] = 0.90f; c.surface0[2] = 0.92f; c.surface0[3] = 1.0f;
            c.surface1[0] = 0.85f; c.surface1[1] = 0.85f; c.surface1[2] = 0.87f; c.surface1[3] = 1.0f;
            c.surface2[0] = 0.78f; c.surface2[1] = 0.78f; c.surface2[2] = 0.80f; c.surface2[3] = 1.0f;
            c.surface3[0] = 0.70f; c.surface3[1] = 0.70f; c.surface3[2] = 0.72f; c.surface3[3] = 1.0f;

            c.accent[0] = 0.20f; c.accent[1] = 0.45f; c.accent[2] = 0.85f; c.accent[3] = 1.0f;
            c.accentLight[0] = 0.30f; c.accentLight[1] = 0.55f; c.accentLight[2] = 0.95f; c.accentLight[3] = 1.0f;
            c.accentDim[0] = 0.15f; c.accentDim[1] = 0.35f; c.accentDim[2] = 0.70f; c.accentDim[3] = 1.0f;

            c.pageBg[0] = 1.00f; c.pageBg[1] = 1.00f; c.pageBg[2] = 1.00f; c.pageBg[3] = 1.0f;
            c.pageText[0] = 0.10f; c.pageText[1] = 0.10f; c.pageText[2] = 0.10f; c.pageText[3] = 1.0f;
            c.pageBorder[0] = 0.75f; c.pageBorder[1] = 0.75f; c.pageBorder[2] = 0.75f; c.pageBorder[3] = 0.80f;
            c.spineShadow[0] = 0.20f; c.spineShadow[1] = 0.20f; c.spineShadow[2] = 0.20f; c.spineShadow[3] = 0.20f;

            c.textPrimary[0] = 0.12f; c.textPrimary[1] = 0.12f; c.textPrimary[2] = 0.12f; c.textPrimary[3] = 1.0f;
            c.textDim[0] = 0.35f; c.textDim[1] = 0.35f; c.textDim[2] = 0.35f; c.textDim[3] = 1.0f;
            c.textFaint[0] = 0.55f; c.textFaint[1] = 0.55f; c.textFaint[2] = 0.55f; c.textFaint[3] = 1.0f;

            c.border[0] = 0.78f; c.border[1] = 0.78f; c.border[2] = 0.80f; c.border[3] = 0.60f;
            c.borderFaint[0] = 0.85f; c.borderFaint[1] = 0.85f; c.borderFaint[2] = 0.87f; c.borderFaint[3] = 0.50f;
            break;
        }

        case ThemePreset::SepiaWarm: {
            // Sepia Suave
            c.base[0] = 0.16f; c.base[1] = 0.13f; c.base[2] = 0.10f; c.base[3] = 1.0f;
            c.surface0[0] = 0.20f; c.surface0[1] = 0.16f; c.surface0[2] = 0.13f; c.surface0[3] = 1.0f;
            c.surface1[0] = 0.25f; c.surface1[1] = 0.20f; c.surface1[2] = 0.16f; c.surface1[3] = 1.0f;
            c.surface2[0] = 0.31f; c.surface2[1] = 0.25f; c.surface2[2] = 0.20f; c.surface2[3] = 1.0f;
            c.surface3[0] = 0.38f; c.surface3[1] = 0.31f; c.surface3[2] = 0.25f; c.surface3[3] = 1.0f;

            c.accent[0] = 0.82f; c.accent[1] = 0.54f; c.accent[2] = 0.24f; c.accent[3] = 1.0f;
            c.accentLight[0] = 0.92f; c.accentLight[1] = 0.68f; c.accentLight[2] = 0.38f; c.accentLight[3] = 1.0f;
            c.accentDim[0] = 0.58f; c.accentDim[1] = 0.36f; c.accentDim[2] = 0.15f; c.accentDim[3] = 1.0f;

            c.pageBg[0] = 0.92f; c.pageBg[1] = 0.85f; c.pageBg[2] = 0.74f; c.pageBg[3] = 1.0f;
            c.pageText[0] = 0.18f; c.pageText[1] = 0.12f; c.pageText[2] = 0.08f; c.pageText[3] = 1.0f;
            c.pageBorder[0] = 0.55f; c.pageBorder[1] = 0.42f; c.pageBorder[2] = 0.28f; c.pageBorder[3] = 0.60f;
            c.spineShadow[0] = 0.12f; c.spineShadow[1] = 0.08f; c.spineShadow[2] = 0.04f; c.spineShadow[3] = 0.50f;

            c.textPrimary[0] = 0.92f; c.textPrimary[1] = 0.86f; c.textPrimary[2] = 0.78f; c.textPrimary[3] = 1.0f;
            c.textDim[0] = 0.74f; c.textDim[1] = 0.67f; c.textDim[2] = 0.58f; c.textDim[3] = 1.0f;
            c.textFaint[0] = 0.50f; c.textFaint[1] = 0.44f; c.textFaint[2] = 0.38f; c.textFaint[3] = 1.0f;

            c.border[0] = 0.40f; c.border[1] = 0.32f; c.border[2] = 0.24f; c.border[3] = 0.40f;
            c.borderFaint[0] = 0.28f; c.borderFaint[1] = 0.22f; c.borderFaint[2] = 0.16f; c.borderFaint[3] = 0.25f;
            break;
        }
    }

    return c;
}

void SettingsManager::ApplyTheme() {
    ThemeColors c = GetCurrentThemeColors();
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 6.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding      = 5.0f;
    style.TabRounding       = 6.0f;

    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;

    style.WindowPadding     = ImVec2(12.0f, 12.0f);
    style.FramePadding      = ImVec2(8.0f, 6.0f);
    style.ItemSpacing       = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing  = ImVec2(6.0f, 6.0f);

    auto toVec4 = [](const float v[4], float aMult = 1.0f) {
        return ImVec4(v[0], v[1], v[2], v[3] * aMult);
    };

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                  = toVec4(c.textPrimary);
    colors[ImGuiCol_TextDisabled]          = toVec4(c.textFaint);
    colors[ImGuiCol_WindowBg]              = toVec4(c.base);
    colors[ImGuiCol_ChildBg]               = toVec4(c.surface0);
    colors[ImGuiCol_PopupBg]               = toVec4(c.surface1);
    colors[ImGuiCol_Border]                = toVec4(c.border);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);

    colors[ImGuiCol_FrameBg]               = toVec4(c.surface1);
    colors[ImGuiCol_FrameBgHovered]        = toVec4(c.surface2);
    colors[ImGuiCol_FrameBgActive]         = toVec4(c.surface3);

    colors[ImGuiCol_TitleBg]               = toVec4(c.surface0);
    colors[ImGuiCol_TitleBgActive]         = toVec4(c.surface1);
    colors[ImGuiCol_TitleBgCollapsed]      = toVec4(c.surface0, 0.7f);

    colors[ImGuiCol_MenuBarBg]             = toVec4(c.surface0);
    colors[ImGuiCol_ScrollbarBg]           = toVec4(c.surface0);
    colors[ImGuiCol_ScrollbarGrab]         = toVec4(c.surface2);
    colors[ImGuiCol_ScrollbarGrabHovered]  = toVec4(c.surface3);
    colors[ImGuiCol_ScrollbarGrabActive]   = toVec4(c.accent);

    colors[ImGuiCol_CheckMark]             = toVec4(c.accentLight);
    colors[ImGuiCol_SliderGrab]            = toVec4(c.accent);
    colors[ImGuiCol_SliderGrabActive]      = toVec4(c.accentLight);

    colors[ImGuiCol_Button]                = toVec4(c.surface1);
    colors[ImGuiCol_ButtonHovered]         = toVec4(c.surface2);
    colors[ImGuiCol_ButtonActive]          = toVec4(c.surface3);

    colors[ImGuiCol_Header]                = toVec4(c.surface1);
    colors[ImGuiCol_HeaderHovered]         = toVec4(c.surface2);
    colors[ImGuiCol_HeaderActive]          = toVec4(c.surface3);

    colors[ImGuiCol_Separator]             = toVec4(c.border);
    colors[ImGuiCol_SeparatorHovered]      = toVec4(c.accentDim);
    colors[ImGuiCol_SeparatorActive]       = toVec4(c.accent);

    colors[ImGuiCol_ResizeGrip]            = toVec4(c.surface2);
    colors[ImGuiCol_ResizeGripHovered]     = toVec4(c.accentDim);
    colors[ImGuiCol_ResizeGripActive]      = toVec4(c.accent);

    colors[ImGuiCol_Tab]                   = toVec4(c.surface0);
    colors[ImGuiCol_TabHovered]            = toVec4(c.surface2);
    colors[ImGuiCol_TabActive]             = toVec4(c.surface1);
    colors[ImGuiCol_TabUnfocused]          = toVec4(c.surface0);
    colors[ImGuiCol_TabUnfocusedActive]   = toVec4(c.surface1);

    colors[ImGuiCol_DockingPreview]        = toVec4(c.accent, 0.6f);
    colors[ImGuiCol_DockingEmptyBg]        = toVec4(c.base);

    colors[ImGuiCol_TableHeaderBg]         = toVec4(c.surface1);
    colors[ImGuiCol_TableBorderStrong]     = toVec4(c.border);
    colors[ImGuiCol_TableBorderLight]      = toVec4(c.borderFaint);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_TableRowBgAlt]         = toVec4(c.surface0, 0.4f);

    colors[ImGuiCol_TextSelectedBg]        = toVec4(c.accent, 0.35f);
    colors[ImGuiCol_DragDropTarget]        = toVec4(c.accentLight);
    colors[ImGuiCol_NavHighlight]          = toVec4(c.accent);
}

void SettingsManager::Save() {
    std::string path = AppPaths::GetConfigFilePath();
    json j;
    j["theme"]               = static_cast<int>(m_Settings.theme);
    j["readerMode"]          = static_cast<int>(m_Settings.readerMode);
    j["workspaceView"]       = static_cast<int>(m_Settings.workspaceView);
    j["readerFontSize"]      = m_Settings.readerFontSize;
    j["readerLineSpacing"]   = m_Settings.readerLineSpacing;
    j["readerPageMargin"]    = m_Settings.readerPageMargin;
    j["readerFontIndex"]     = m_Settings.readerFontIndex;
    j["showDropCaps"]        = m_Settings.showDropCaps;
    j["showPageBorders"]     = m_Settings.showPageBorders;
    j["showSpineShadow"]     = m_Settings.showSpineShadow;
    j["showMarginalia"]      = m_Settings.showMarginalia;
    j["showBookmarksRibbon"] = m_Settings.showBookmarksRibbon;
    j["autoSave"]            = m_Settings.autoSave;
    j["autoSaveIntervalSec"] = m_Settings.autoSaveIntervalSec;
    j["showLibraryPanel"]    = m_Settings.showLibraryPanel;
    j["showTocPanel"]        = m_Settings.showTocPanel;
    j["showSettingsPanel"]   = m_Settings.showSettingsPanel;

    // Actualizaciones
    j["checkUpdatesOnStartup"] = m_Settings.checkUpdatesOnStartup;
    j["updateChannel"]         = m_Settings.updateChannel;
    j["githubRepo"]            = m_Settings.githubRepo;
    j["githubToken"]           = m_Settings.githubToken;
    j["lastUpdateCheck"]       = m_Settings.lastUpdateCheck;

    std::ofstream out(path);
    if (out.is_open()) {
        out << j.dump(4);
    }
}

void SettingsManager::Load() {
    std::string path = AppPaths::GetConfigFilePath();
    std::ifstream in(path);
    if (!in.is_open()) return;

    try {
        json j;
        in >> j;
        if (j.contains("theme")) m_Settings.theme = static_cast<ThemePreset>(j["theme"].get<int>());
        if (j.contains("readerMode")) m_Settings.readerMode = static_cast<ReaderMode>(j["readerMode"].get<int>());
        if (j.contains("workspaceView")) m_Settings.workspaceView = static_cast<WorkspaceView>(j["workspaceView"].get<int>());
        if (j.contains("readerFontSize")) m_Settings.readerFontSize = j["readerFontSize"].get<float>();
        if (j.contains("readerLineSpacing")) m_Settings.readerLineSpacing = j["readerLineSpacing"].get<float>();
        if (j.contains("readerPageMargin")) m_Settings.readerPageMargin = j["readerPageMargin"].get<float>();
        if (j.contains("readerFontIndex")) m_Settings.readerFontIndex = j["readerFontIndex"].get<int>();
        if (j.contains("showDropCaps")) m_Settings.showDropCaps = j["showDropCaps"].get<bool>();
        if (j.contains("showPageBorders")) m_Settings.showPageBorders = j["showPageBorders"].get<bool>();
        if (j.contains("showSpineShadow")) m_Settings.showSpineShadow = j["showSpineShadow"].get<bool>();
        if (j.contains("showMarginalia")) m_Settings.showMarginalia = j["showMarginalia"].get<bool>();
        if (j.contains("showBookmarksRibbon")) m_Settings.showBookmarksRibbon = j["showBookmarksRibbon"].get<bool>();
        if (j.contains("autoSave")) m_Settings.autoSave = j["autoSave"].get<bool>();
        if (j.contains("autoSaveIntervalSec")) m_Settings.autoSaveIntervalSec = j["autoSaveIntervalSec"].get<int>();
        if (j.contains("showLibraryPanel")) m_Settings.showLibraryPanel = j["showLibraryPanel"].get<bool>();
        if (j.contains("showTocPanel")) m_Settings.showTocPanel = j["showTocPanel"].get<bool>();
        if (j.contains("showSettingsPanel")) m_Settings.showSettingsPanel = j["showSettingsPanel"].get<bool>();

        if (j.contains("checkUpdatesOnStartup")) m_Settings.checkUpdatesOnStartup = j["checkUpdatesOnStartup"].get<bool>();
        if (j.contains("updateChannel")) m_Settings.updateChannel = j["updateChannel"].get<std::string>();
        if (j.contains("githubRepo")) m_Settings.githubRepo = j["githubRepo"].get<std::string>();
        if (j.contains("githubToken")) m_Settings.githubToken = j["githubToken"].get<std::string>();
        if (j.contains("lastUpdateCheck")) m_Settings.lastUpdateCheck = j["lastUpdateCheck"].get<std::string>();
    } catch (...) {
        // Fallback a defaults
    }
}

} // namespace WitcherNotes::Settings
