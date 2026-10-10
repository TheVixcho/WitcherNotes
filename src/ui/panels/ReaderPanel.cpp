#include "ReaderPanel.h"
#include "core/BookModel.h"
#include "core/SettingsManager.h"
#include <imgui_internal.h>
#include <algorithm>
#include <sstream>
#include <cmath>

namespace WitcherNotes::UI {

using namespace WitcherNotes::Core;
using namespace WitcherNotes::Settings;

static ImU32 ToU32(const float c[4], float alpha = -1.0f) {
    auto toB = [](float v) {
        v = std::clamp(v, 0.0f, 1.0f);
        return (int)(v * 255.0f + 0.5f);
    };
    float a = (alpha >= 0.0f) ? alpha : c[3];
    return IM_COL32(toB(c[0]), toB(c[1]), toB(c[2]), toB(a));
}

static ImVec4 ToVec4(const float c[4], float alpha = -1.0f) {
    float a = (alpha >= 0.0f) ? alpha : c[3];
    return ImVec4(c[0], c[1], c[2], a);
}

ReaderPanel::ReaderPanel() {}

void ReaderPanel::Render() {
    auto& repo = BookRepository::Get();
    auto& settings = SettingsManager::Get().GetSettings();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    Book* book = repo.GetActiveBook();
    if (!book || book->chapters.empty()) {
        ImGui::BeginChild("##ReaderEmpty", ImVec2(0, 0), true);
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.4f);
        ImGui::TextColored(ToVec4(theme.accentLight), "    No document selected.");
        ImGui::TextColored(ToVec4(theme.textDim), "    Create or select a notebook from the library to begin reading or editing.");
        ImGui::EndChild();
        return;
    }

    // Top control toolbar
    RenderControlsBar();

    // Main reading area
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x < 50.0f || avail.y < 50.0f) return;

    // Keyboard shortcuts for page turn (Left / Right)
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_PageUp)) {
            if (repo.HasPrevPage()) repo.PrevPage();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_PageDown) || ImGui::IsKeyPressed(ImGuiKey_Space)) {
            if (repo.HasNextPage()) repo.NextPage();
        }
    }

    if (settings.readerMode == ReaderMode::TwoPageSpread && avail.x > 750.0f) {
        RenderTwoPageSpread(avail.x, avail.y);
    } else if (settings.readerMode == ReaderMode::ContinuousScroll) {
        RenderContinuousScroll(avail.x, avail.y);
    } else {
        RenderSinglePage(avail.x, avail.y);
    }
}

void ReaderPanel::RenderControlsBar() {
    auto& repo = BookRepository::Get();
    auto& settings = SettingsManager::Get().GetSettings();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();
    Book* book = repo.GetActiveBook();
    Chapter* ch = repo.GetActiveChapter();
    Page* pg = repo.GetActivePage();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 4));

    // Page navigation buttons
    bool hasPrev = repo.HasPrevPage();
    bool hasNext = repo.HasNextPage();

    if (!hasPrev) ImGui::BeginDisabled();
    if (ImGui::Button("Previous")) {
        repo.PrevPage();
    }
    if (!hasPrev) ImGui::EndDisabled();

    ImGui::SameLine();
    if (!hasNext) ImGui::BeginDisabled();
    if (ImGui::Button("Next")) {
        repo.NextPage();
    }
    if (!hasNext) ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    // Reading mode selector
    const char* modeNames[] = { "Two Pages", "Single Page", "Continuous Scroll" };
    int curMode = static_cast<int>(settings.readerMode);
    ImGui::SetNextItemWidth(150.0f);
    if (ImGui::Combo("##ReaderModeCombo", &curMode, modeNames, 3)) {
        settings.readerMode = static_cast<ReaderMode>(curMode);
        SettingsManager::Get().Save();
    }

    ImGui::SameLine();
    // Font zoom
    if (ImGui::Button("A-") && settings.readerFontSize > 12.0f) {
        settings.readerFontSize -= 1.0f;
        SettingsManager::Get().Save();
    }
    ImGui::SameLine();
    if (ImGui::Button("A+") && settings.readerFontSize < 32.0f) {
        settings.readerFontSize += 1.0f;
        SettingsManager::Get().Save();
    }

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    // Bookmark
    if (pg) {
        bool bmk = pg->isBookmarked;
        if (bmk) {
            ImGui::PushStyleColor(ImGuiCol_Button, ToVec4(theme.accent));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.08f, 0.05f, 1.0f));
        }
        if (ImGui::Button(bmk ? "Bookmarked" : "Bookmark")) {
            pg->isBookmarked = !pg->isBookmarked;
            repo.MarkDirty();
            repo.Save();
        }
        if (bmk) {
            ImGui::PopStyleColor(2);
        }
    }

    // Reading stats on right
    if (book && ch && pg) {
        int totalBookPages = book->GetTotalPages();
        int curGlobalPage = book->GetGlobalPageIndex(repo.GetActiveChapterIndex(), repo.GetActivePageIndex()) + 1;
        int wordCount = pg->GetWordCount();
        int readTime = pg->GetReadingTimeMinutes();

        char infoBuf[128];
        snprintf(infoBuf, sizeof(infoBuf), "Page %d of %d  |  %d words (~%d min)",
                 curGlobalPage, totalBookPages, wordCount, readTime);

        float textW = ImGui::CalcTextSize(infoBuf).x;
        float remainX = ImGui::GetContentRegionAvail().x;
        if (remainX > textW + 10.0f) {
            ImGui::SameLine(ImGui::GetWindowWidth() - textW - 20.0f);
            ImGui::TextColored(ToVec4(theme.textDim), "%s", infoBuf);
        }
    }

    ImGui::PopStyleVar(2);
    ImGui::Separator();
}

void ReaderPanel::RenderTwoPageSpread(float availWidth, float availHeight) {
    auto& repo = BookRepository::Get();
    Book* book = repo.GetActiveBook();
    Chapter* ch = repo.GetActiveChapter();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    float bookPad = 12.0f;
    float pageW = (availWidth - (bookPad * 2.0f) - 16.0f) * 0.5f;
    float pageH = availHeight - 14.0f;

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Book outer binding
    ImVec2 bookMin(origin.x + bookPad - 6.0f, origin.y + 2.0f);
    ImVec2 bookMax(origin.x + (bookPad * 2.0f) + (pageW * 2.0f) + 12.0f, origin.y + pageH + 6.0f);
    dl->AddRectFilled(bookMin, bookMax, ToU32(theme.surface0), 10.0f);
    dl->AddRect(bookMin, bookMax, ToU32(theme.accent, 0.4f), 10.0f, 0, 1.5f);

    // Left Page (Active page)
    int leftChIdx = repo.GetActiveChapterIndex();
    int leftPgIdx = repo.GetActivePageIndex();
    int totalBookPages = book->GetTotalPages();
    int leftGlobalPage = book->GetGlobalPageIndex(leftChIdx, leftPgIdx) + 1;

    Page* leftPage = repo.GetActivePage();
    ImVec2 leftPos(origin.x + bookPad, origin.y + 6.0f);
    ImVec2 pageSize(pageW, pageH - 4.0f);

    if (leftPage) {
        RenderPageContent(leftPage->title, leftPage->content, leftPage->marginalia,
                          leftPage->signTag, leftGlobalPage, totalBookPages,
                          leftPos, pageSize, true);
    }

    // Right Page (Next page if exists)
    int rightChIdx = leftChIdx;
    int rightPgIdx = leftPgIdx + 1;
    Page* rightPage = nullptr;

    if (ch && rightPgIdx < (int)ch->pages.size()) {
        rightPage = &ch->pages[rightPgIdx];
    } else if (leftChIdx + 1 < (int)book->chapters.size() && !book->chapters[leftChIdx + 1].pages.empty()) {
        rightChIdx = leftChIdx + 1;
        rightPgIdx = 0;
        rightPage = &book->chapters[rightChIdx].pages[0];
    }

    ImVec2 rightPos(origin.x + bookPad + pageW + 6.0f, origin.y + 6.0f);
    int rightGlobalPage = leftGlobalPage + 1;

    if (rightPage) {
        RenderPageContent(rightPage->title, rightPage->content, rightPage->marginalia,
                          rightPage->signTag, rightGlobalPage, totalBookPages,
                          rightPos, pageSize, false);
    } else {
        // Blank end of book page
        dl->AddRectFilled(rightPos, ImVec2(rightPos.x + pageSize.x, rightPos.y + pageSize.y),
                          ToU32(theme.pageBg), 6.0f);
        dl->AddRect(rightPos, ImVec2(rightPos.x + pageSize.x, rightPos.y + pageSize.y),
                    ToU32(theme.pageBorder), 6.0f);

        ImVec2 textPos(rightPos.x + pageSize.x * 0.35f, rightPos.y + pageSize.y * 0.45f);
        dl->AddText(textPos, ToU32(theme.textFaint), "— End of Document —");
    }

    // Sombra del lomo central (Efecto encuadernación física)
    if (SettingsManager::Get().GetSettings().showSpineShadow) {
        float spineX = origin.x + bookPad + pageW + 3.0f;
        float shadowW = 18.0f;
        dl->AddRectFilledMultiColor(
            ImVec2(spineX - shadowW, origin.y + 6.0f),
            ImVec2(spineX, origin.y + pageH + 2.0f),
            IM_COL32(0, 0, 0, 0), ToU32(theme.spineShadow),
            ToU32(theme.spineShadow), IM_COL32(0, 0, 0, 0));

        dl->AddRectFilledMultiColor(
            ImVec2(spineX, origin.y + 6.0f),
            ImVec2(spineX + shadowW, origin.y + pageH + 2.0f),
            ToU32(theme.spineShadow), IM_COL32(0, 0, 0, 0),
            IM_COL32(0, 0, 0, 0), ToU32(theme.spineShadow));
    }

    ImGui::Dummy(ImVec2(availWidth, availHeight));
}

void ReaderPanel::RenderSinglePage(float availWidth, float availHeight) {
    auto& repo = BookRepository::Get();
    Book* book = repo.GetActiveBook();
    Page* page = repo.GetActivePage();
    if (!book || !page) return;

    float maxW = std::min(availWidth - 40.0f, 820.0f);
    float padX = (availWidth - maxW) * 0.5f;
    float pageH = availHeight - 16.0f;

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImVec2 pos(origin.x + padX, origin.y + 8.0f);
    ImVec2 size(maxW, pageH);

    int totalPages = book->GetTotalPages();
    int globalPage = book->GetGlobalPageIndex(repo.GetActiveChapterIndex(), repo.GetActivePageIndex()) + 1;

    RenderPageContent(page->title, page->content, page->marginalia, page->signTag,
                      globalPage, totalPages, pos, size, false);

    ImGui::Dummy(ImVec2(availWidth, availHeight));
}

void ReaderPanel::RenderContinuousScroll(float availWidth, float availHeight) {
    auto& repo = BookRepository::Get();
    Book* book = repo.GetActiveBook();
    Chapter* ch = repo.GetActiveChapter();
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();

    if (!ch) return;

    ImGui::BeginChild("##ContinuousScrollRegion", ImVec2(availWidth, availHeight), false);

    float maxW = std::min(availWidth - 60.0f, 840.0f);
    float padX = (availWidth - maxW) * 0.5f;

    ImGui::SetCursorPosX(padX);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ToVec4(theme.pageBg));
    ImGui::PushStyleColor(ImGuiCol_Border, ToVec4(theme.pageBorder));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(36.0f, 36.0f));

    ImGui::BeginChild("##ScrollSheet", ImVec2(maxW, 0), true, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::PushStyleColor(ImGuiCol_Text, ToVec4(theme.pageText));

    // Chapter header
    ImGui::TextColored(ToVec4(theme.accent), "SECTION: %s", ch->title.c_str());
    if (!ch->description.empty()) {
        ImGui::TextColored(ToVec4(theme.textDim), "%s", ch->description.c_str());
    }
    ImGui::Separator();
    ImGui::Spacing();

    for (size_t i = 0; i < ch->pages.size(); ++i) {
        const auto& pg = ch->pages[i];

        // Page title and category badge
        ImGui::Spacing();
        std::string badge = (!pg.signTag.empty() && pg.signTag != "GENERAL") ? ("[" + pg.signTag + "] ") : "";
        ImGui::TextColored(ToVec4(theme.accentLight), "%s%s", badge.c_str(), pg.title.c_str());
        ImGui::Spacing();

        // Wrapped content
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + maxW - 72.0f);
        ImGui::TextUnformatted(pg.content.c_str());
        ImGui::PopTextWrapPos();

        if (!pg.marginalia.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ToVec4(theme.accentDim), "  Marginalia: %s", pg.marginalia.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
    }

    ImGui::PopStyleColor(); // ImGuiCol_Text
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    ImGui::EndChild();
}

void ReaderPanel::RenderPageContent(const std::string& title, const std::string& content,
                                    const std::string& marginalia, const std::string& signTag,
                                    int pageNumber, int totalPages,
                                    const ImVec2& pos, const ImVec2& size, bool isLeftPage)
{
    ThemeColors theme = SettingsManager::Get().GetCurrentThemeColors();
    auto& settings = SettingsManager::Get().GetSettings();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Page shadow
    ImVec2 pageMin = pos;
    ImVec2 pageMax(pos.x + size.x, pos.y + size.y);

    dl->AddRectFilled(ImVec2(pageMin.x + 2.0f, pageMin.y + 2.0f),
                      ImVec2(pageMax.x + 2.0f, pageMax.y + 2.0f),
                      IM_COL32(0, 0, 0, 45), 6.0f);

    // Page background
    dl->AddRectFilled(pageMin, pageMax, ToU32(theme.pageBg), 6.0f);

    // Page decorative borders
    if (settings.showPageBorders) {
        dl->AddRect(pageMin, pageMax, ToU32(theme.pageBorder), 6.0f, 0, 1.2f);
        dl->AddRect(ImVec2(pageMin.x + 5.0f, pageMin.y + 5.0f),
                    ImVec2(pageMax.x - 5.0f, pageMax.y - 5.0f),
                    ToU32(theme.pageBorder, 0.35f), 4.0f, 0, 1.0f);
    }

    // Document header
    auto& repo = BookRepository::Get();
    Book* book = repo.GetActiveBook();
    Chapter* ch = repo.GetActiveChapter();

    float headerY = pos.y + 16.0f;
    std::string headerText = isLeftPage ? (book ? book->title : "Document") : (ch ? ch->title : "Section");

    ImVec2 headerSz = ImGui::CalcTextSize(headerText.c_str());
    float headerX = pos.x + (size.x - headerSz.x) * 0.5f;
    dl->AddText(ImVec2(headerX, headerY), ToU32(theme.textFaint), headerText.c_str());

    // Category badge in upper corner
    if (!signTag.empty() && signTag != "GENERAL") {
        std::string badge = "[" + signTag + "]";
        ImVec2 badgeSz = ImGui::CalcTextSize(badge.c_str());
        float badgeX = isLeftPage ? (pos.x + 22.0f) : (pos.x + size.x - badgeSz.x - 22.0f);
        dl->AddText(ImVec2(badgeX, headerY), ToU32(theme.accent), badge.c_str());
    }

    // Divider under header
    dl->AddLine(ImVec2(pos.x + 24.0f, headerY + 18.0f),
                ImVec2(pos.x + size.x - 24.0f, headerY + 18.0f),
                ToU32(theme.pageBorder, 0.25f), 1.0f);

    // Page text scroll container
    float textStartY = headerY + 28.0f;
    float footerH = 34.0f;
    float textH = size.y - (textStartY - pos.y) - footerH;
    float marginX = settings.readerPageMargin;
    float textW = size.x - (marginX * 2.0f);

    // Marginalia margin if enabled
    bool hasMarginalia = settings.showMarginalia && !marginalia.empty();
    float marginaliaW = hasMarginalia ? std::min(textW * 0.28f, 150.0f) : 0.0f;
    float contentW = textW - marginaliaW - (hasMarginalia ? 14.0f : 0.0f);

    std::string childId = "##PageChild_" + std::to_string(pageNumber);
    ImGui::SetCursorScreenPos(ImVec2(pos.x + marginX, textStartY));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::BeginChild(childId.c_str(), ImVec2(textW, textH), false);

    ImGui::PushStyleColor(ImGuiCol_Text, ToVec4(theme.pageText));

    // Page title
    ImGui::Spacing();
    ImGui::TextColored(ToVec4(theme.accentLight), "%s", title.c_str());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Body text
    if (hasMarginalia) {
        ImGui::Columns(2, "##MarginaliaCols", false);
        ImGui::SetColumnWidth(0, contentW);
        ImGui::SetColumnWidth(1, marginaliaW);

        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + contentW);
        ImGui::TextUnformatted(content.c_str());
        ImGui::PopTextWrapPos();

        ImGui::NextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, ToVec4(theme.accentDim));
        ImGui::Text("Notes:");
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + marginaliaW);
        ImGui::TextWrapped("%s", marginalia.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();

        ImGui::Columns(1);
    } else {
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + contentW);
        ImGui::TextUnformatted(content.c_str());
        ImGui::PopTextWrapPos();
    }

    ImGui::PopStyleColor(); // ImGuiCol_Text
    ImGui::EndChild();
    ImGui::PopStyleColor(); // ImGuiCol_ChildBg

    // Pie de página (Footer) con número de página
    float footerY = pos.y + size.y - 24.0f;
    char pageStr[32];
    snprintf(pageStr, sizeof(pageStr), "%d", pageNumber);
    ImVec2 pageSz = ImGui::CalcTextSize(pageStr);
    float pageNumX = pos.x + (size.x - pageSz.x) * 0.5f;

    dl->AddLine(ImVec2(pos.x + 24.0f, footerY - 8.0f),
                ImVec2(pos.x + size.x - 24.0f, footerY - 8.0f),
                ToU32(theme.pageBorder, 0.25f), 1.0f);
    dl->AddText(ImVec2(pageNumX, footerY), ToU32(theme.textFaint), pageStr);
}

} // namespace WitcherNotes::UI
