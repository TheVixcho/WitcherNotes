#include "BookModel.h"
#include "AppPaths.h"
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>

using json = nlohmann::json;

namespace WitcherNotes::Core {

static std::string GetCurrentTimestampStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M");
    return oss.str();
}

std::string CategoryToString(NoteCategory cat) {
    switch (cat) {
        case NoteCategory::Ideas:       return "IDEAS";
        case NoteCategory::Borrador:    return "DRAFT";
        case NoteCategory::Importante:  return "IMPORTANT";
        case NoteCategory::Referencia:  return "REFERENCE";
        case NoteCategory::Tareas:      return "TASKS";
        case NoteCategory::Proyecto:    return "PROJECT";
        case NoteCategory::Estudio:     return "STUDY";
        default:                        return "GENERAL";
    }
}

NoteCategory StringToCategory(const std::string& str) {
    std::string upper = str;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    if (upper == "IDEAS")                                         return NoteCategory::Ideas;
    if (upper == "DRAFT" || upper == "BORRADOR")                  return NoteCategory::Borrador;
    if (upper == "IMPORTANT" || upper == "IMPORTANTE" || upper == "PRIORITY") return NoteCategory::Importante;
    if (upper == "REFERENCE" || upper == "REFERENCIA")            return NoteCategory::Referencia;
    if (upper == "TASKS" || upper == "TAREAS")                    return NoteCategory::Tareas;
    if (upper == "PROJECT" || upper == "PROYECTO")                return NoteCategory::Proyecto;
    if (upper == "STUDY" || upper == "ESTUDIO")                   return NoteCategory::Estudio;
    return NoteCategory::General;
}

const char* CategoryDisplayName(NoteCategory cat) {
    switch (cat) {
        case NoteCategory::Ideas:       return "Ideas & Brainstorming";
        case NoteCategory::Borrador:    return "Draft";
        case NoteCategory::Importante:  return "Priority / Important";
        case NoteCategory::Referencia:  return "Reference & Resources";
        case NoteCategory::Tareas:      return "Tasks & Actions";
        case NoteCategory::Proyecto:    return "Project Documentation";
        case NoteCategory::Estudio:     return "Study & Research";
        default:                        return "General Note";
    }
}

const char* CategorySymbol(NoteCategory cat) {
    switch (cat) {
        case NoteCategory::Ideas:       return "[IDEAS]";
        case NoteCategory::Borrador:    return "[DRAFT]";
        case NoteCategory::Importante:  return "[IMPORTANT]";
        case NoteCategory::Referencia:  return "[REFERENCE]";
        case NoteCategory::Tareas:      return "[TASKS]";
        case NoteCategory::Proyecto:    return "[PROJECT]";
        case NoteCategory::Estudio:     return "[STUDY]";
        default:                        return "[GENERAL]";
    }
}

// ---------------- Page ----------------
int Page::GetWordCount() const {
    if (content.empty()) return 0;
    std::istringstream iss(content);
    std::string token;
    int count = 0;
    while (iss >> token) {
        count++;
    }
    return count;
}

int Page::GetReadingTimeMinutes() const {
    int words = GetWordCount();
    int mins = words / 200; // ~200 palabras por minuto
    return (mins < 1) ? 1 : mins;
}

// ---------------- Chapter ----------------
int Chapter::GetTotalWords() const {
    int total = 0;
    for (const auto& p : pages) {
        total += p.GetWordCount();
    }
    return total;
}

// ---------------- Book ----------------
int Book::GetTotalPages() const {
    int total = 0;
    for (const auto& ch : chapters) {
        total += (int)ch.pages.size();
    }
    return total;
}

int Book::GetTotalWords() const {
    int total = 0;
    for (const auto& ch : chapters) {
        total += ch.GetTotalWords();
    }
    return total;
}

int Book::GetGlobalPageIndex(int chapterIdx, int pageIdx) const {
    int idx = 0;
    for (int c = 0; c < chapterIdx && c < (int)chapters.size(); ++c) {
        idx += (int)chapters[c].pages.size();
    }
    return idx + pageIdx;
}

bool Book::GetChapterAndPageFromGlobalIndex(int globalIdx, int& outChapterIdx, int& outPageIdx) const {
    int current = 0;
    for (int c = 0; c < (int)chapters.size(); ++c) {
        int count = (int)chapters[c].pages.size();
        if (globalIdx < current + count) {
            outChapterIdx = c;
            outPageIdx = globalIdx - current;
            return true;
        }
        current += count;
    }
    return false;
}

// ---------------- BookRepository ----------------
BookRepository::BookRepository() {
    PopulateDefaultsIfEmpty();
}

BookRepository& BookRepository::Get() {
    static BookRepository instance;
    return instance;
}

void BookRepository::SetActiveIndices(int bookIdx, int chapterIdx, int pageIdx) {
    if (m_Books.empty()) return;
    m_ActiveBookIdx = std::clamp(bookIdx, 0, (int)m_Books.size() - 1);
    auto& b = m_Books[m_ActiveBookIdx];

    if (b.chapters.empty()) {
        m_ActiveChapterIdx = 0;
        m_ActivePageIdx = 0;
        return;
    }
    m_ActiveChapterIdx = std::clamp(chapterIdx, 0, (int)b.chapters.size() - 1);
    auto& ch = b.chapters[m_ActiveChapterIdx];

    if (ch.pages.empty()) {
        m_ActivePageIdx = 0;
        return;
    }
    m_ActivePageIdx = std::clamp(pageIdx, 0, (int)ch.pages.size() - 1);
}

void BookRepository::SetActivePage(int pageIdx) {
    SetActiveIndices(m_ActiveBookIdx, m_ActiveChapterIdx, pageIdx);
}

void BookRepository::SetActiveChapter(int chapterIdx) {
    SetActiveIndices(m_ActiveBookIdx, chapterIdx, 0);
}

void BookRepository::SetActiveBook(int bookIdx) {
    SetActiveIndices(bookIdx, 0, 0);
}

Book* BookRepository::GetActiveBook() {
    if (m_Books.empty()) return nullptr;
    if (m_ActiveBookIdx < 0 || m_ActiveBookIdx >= (int)m_Books.size()) m_ActiveBookIdx = 0;
    return &m_Books[m_ActiveBookIdx];
}

Chapter* BookRepository::GetActiveChapter() {
    Book* b = GetActiveBook();
    if (!b || b->chapters.empty()) return nullptr;
    if (m_ActiveChapterIdx < 0 || m_ActiveChapterIdx >= (int)b->chapters.size()) m_ActiveChapterIdx = 0;
    return &b->chapters[m_ActiveChapterIdx];
}

Page* BookRepository::GetActivePage() {
    Chapter* ch = GetActiveChapter();
    if (!ch || ch->pages.empty()) return nullptr;
    if (m_ActivePageIdx < 0 || m_ActivePageIdx >= (int)ch->pages.size()) m_ActivePageIdx = 0;
    return &ch->pages[m_ActivePageIdx];
}

bool BookRepository::HasNextPage() const {
    const Book* b = const_cast<BookRepository*>(this)->GetActiveBook();
    if (!b) return false;
    int curGlobal = b->GetGlobalPageIndex(m_ActiveChapterIdx, m_ActivePageIdx);
    return curGlobal + 1 < b->GetTotalPages();
}

bool BookRepository::HasPrevPage() const {
    const Book* b = const_cast<BookRepository*>(this)->GetActiveBook();
    if (!b) return false;
    int curGlobal = b->GetGlobalPageIndex(m_ActiveChapterIdx, m_ActivePageIdx);
    return curGlobal > 0;
}

void BookRepository::NextPage() {
    Book* b = GetActiveBook();
    if (!b) return;
    int curGlobal = b->GetGlobalPageIndex(m_ActiveChapterIdx, m_ActivePageIdx);
    int nextGlobal = curGlobal + 1;
    int newC = 0, newP = 0;
    if (b->GetChapterAndPageFromGlobalIndex(nextGlobal, newC, newP)) {
        SetActiveIndices(m_ActiveBookIdx, newC, newP);
    }
}

void BookRepository::PrevPage() {
    Book* b = GetActiveBook();
    if (!b) return;
    int curGlobal = b->GetGlobalPageIndex(m_ActiveChapterIdx, m_ActivePageIdx);
    int prevGlobal = curGlobal - 1;
    if (prevGlobal >= 0) {
        int newC = 0, newP = 0;
        if (b->GetChapterAndPageFromGlobalIndex(prevGlobal, newC, newP)) {
            SetActiveIndices(m_ActiveBookIdx, newC, newP);
        }
    }
}

void BookRepository::AddBook(const std::string& title, const std::string& subtitle, const std::string& author, const std::string& coverTheme) {
    Book b;
    b.id = "doc_" + std::to_string(std::time(nullptr));
    b.title = title.empty() ? "New Notebook" : title;
    b.subtitle = subtitle;
    b.author = author.empty() ? "User" : author;
    b.category = "Notebooks";
    b.coverTheme = coverTheme.empty() ? "Dark" : coverTheme;
    b.createdAt = GetCurrentTimestampStr();
    b.updatedAt = b.createdAt;

    Chapter ch;
    ch.id = "ch_1";
    ch.title = "Section 1";
    ch.description = "First section of this notebook";

    Page p;
    p.id = "pg_1";
    p.title = "Page 1";
    p.content = "# " + b.title + "\n\nStart writing your notes here...\n";
    p.marginalia = "";
    p.signTag = "GENERAL";
    p.createdAt = b.createdAt;
    p.updatedAt = b.createdAt;

    ch.pages.push_back(p);
    b.chapters.push_back(ch);

    m_Books.push_back(b);
    SetActiveIndices((int)m_Books.size() - 1, 0, 0);
    m_IsDirty = true;
}

void BookRepository::DeleteBook(int index) {
    if (index >= 0 && index < (int)m_Books.size() && m_Books.size() > 1) {
        m_Books.erase(m_Books.begin() + index);
        m_ActiveBookIdx = std::clamp(m_ActiveBookIdx, 0, (int)m_Books.size() - 1);
        SetActiveIndices(m_ActiveBookIdx, 0, 0);
        m_IsDirty = true;
    }
}

void BookRepository::AddChapter(const std::string& title) {
    Book* b = GetActiveBook();
    if (!b) return;

    Chapter ch;
    ch.id = "ch_" + std::to_string(std::time(nullptr));
    ch.title = title.empty() ? ("Section " + std::to_string(b->chapters.size() + 1)) : title;
    ch.description = "";

    Page p;
    p.id = "pg_" + std::to_string(std::time(nullptr));
    p.title = "Page 1";
    p.content = "# " + ch.title + "\n\nWrite your notes here...\n";
    p.marginalia = "";
    p.signTag = "GENERAL";
    p.createdAt = GetCurrentTimestampStr();
    p.updatedAt = p.createdAt;

    ch.pages.push_back(p);
    b->chapters.push_back(ch);

    SetActiveIndices(m_ActiveBookIdx, (int)b->chapters.size() - 1, 0);
    m_IsDirty = true;
}

void BookRepository::DeleteChapter(int chapterIdx) {
    Book* b = GetActiveBook();
    if (!b || b->chapters.size() <= 1) return;

    if (chapterIdx >= 0 && chapterIdx < (int)b->chapters.size()) {
        b->chapters.erase(b->chapters.begin() + chapterIdx);
        m_ActiveChapterIdx = std::clamp(m_ActiveChapterIdx, 0, (int)b->chapters.size() - 1);
        SetActiveIndices(m_ActiveBookIdx, m_ActiveChapterIdx, 0);
        m_IsDirty = true;
    }
}

void BookRepository::AddPage(const std::string& title, const std::string& signTag) {
    Chapter* ch = GetActiveChapter();
    if (!ch) return;

    Page p;
    p.id = "pg_" + std::to_string(std::time(nullptr));
    p.title = title.empty() ? ("Page " + std::to_string(ch->pages.size() + 1)) : title;
    p.content = "## " + p.title + "\n\nStart typing here...\n";
    p.marginalia = "";
    p.signTag = signTag.empty() ? "GENERAL" : signTag;
    p.createdAt = GetCurrentTimestampStr();
    p.updatedAt = p.createdAt;

    ch->pages.push_back(p);
    SetActiveIndices(m_ActiveBookIdx, m_ActiveChapterIdx, (int)ch->pages.size() - 1);
    m_IsDirty = true;
}

void BookRepository::DeletePage(int pageIdx) {
    Chapter* ch = GetActiveChapter();
    if (!ch || ch->pages.size() <= 1) return;

    if (pageIdx >= 0 && pageIdx < (int)ch->pages.size()) {
        ch->pages.erase(ch->pages.begin() + pageIdx);
        m_ActivePageIdx = std::clamp(m_ActivePageIdx, 0, (int)ch->pages.size() - 1);
        SetActiveIndices(m_ActiveBookIdx, m_ActiveChapterIdx, m_ActivePageIdx);
        m_IsDirty = true;
    }
}

void BookRepository::DuplicateCurrentPage() {
    Chapter* ch = GetActiveChapter();
    Page* p = GetActivePage();
    if (!ch || !p) return;

    Page copy = *p;
    copy.id = "pg_" + std::to_string(std::time(nullptr));
    copy.title += " (Copia)";
    copy.createdAt = GetCurrentTimestampStr();
    copy.updatedAt = copy.createdAt;

    ch->pages.insert(ch->pages.begin() + m_ActivePageIdx + 1, copy);
    SetActiveIndices(m_ActiveBookIdx, m_ActiveChapterIdx, m_ActivePageIdx + 1);
    m_IsDirty = true;
}

std::vector<BookRepository::SearchResult> BookRepository::Search(const std::string& query) const {
    std::vector<SearchResult> results;
    if (query.empty()) return results;

    std::string lowerQ = query;
    std::transform(lowerQ.begin(), lowerQ.end(), lowerQ.begin(), ::tolower);

    for (int b = 0; b < (int)m_Books.size(); ++b) {
        const auto& book = m_Books[b];
        for (int c = 0; c < (int)book.chapters.size(); ++c) {
            const auto& ch = book.chapters[c];
            for (int p = 0; p < (int)ch.pages.size(); ++p) {
                const auto& page = ch.pages[p];

                std::string lTitle = page.title;
                std::string lContent = page.content;
                std::transform(lTitle.begin(), lTitle.end(), lTitle.begin(), ::tolower);
                std::transform(lContent.begin(), lContent.end(), lContent.begin(), ::tolower);

                size_t pos = lContent.find(lowerQ);
                if (pos != std::string::npos || lTitle.find(lowerQ) != std::string::npos) {
                    SearchResult r;
                    r.bookIdx = b;
                    r.chapterIdx = c;
                    r.pageIdx = p;
                    r.bookTitle = book.title;
                    r.chapterTitle = ch.title;
                    r.pageTitle = page.title;

                    if (pos != std::string::npos) {
                        size_t start = (pos > 30) ? (pos - 30) : 0;
                        size_t len = std::min((size_t)80, page.content.size() - start);
                        r.matchSnippet = "..." + page.content.substr(start, len) + "...";
                    } else {
                        r.matchSnippet = "Coincidencia en el título de la página.";
                    }
                    results.push_back(r);
                }
            }
        }
    }
    return results;
}

bool BookRepository::ExportActivePageToMarkdown(const std::string& filepath) {
    Page* p = GetActivePage();
    Book* b = GetActiveBook();
    if (!p || !b) return false;

    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << "# " << p->title << "\n\n";
    out << "**Documento:** " << b->title << " | **Fecha:** " << p->updatedAt << "\n\n";
    out << "---\n\n";
    out << p->content << "\n\n";
    if (!p->marginalia.empty()) {
        out << "### Notas al Margen:\n> " << p->marginalia << "\n\n";
    }
    return true;
}

bool BookRepository::ExportActiveBookToMarkdown(const std::string& filepath) {
    Book* b = GetActiveBook();
    if (!b) return false;

    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << "# " << b->title << "\n";
    if (!b->subtitle.empty()) out << "### *" << b->subtitle << "*\n";
    out << "**Autor:** " << b->author << "\n";
    out << "**Compilado con:** Cuaderno de Notas\n\n";
    out << "---\n\n";

    for (const auto& ch : b->chapters) {
        out << "## " << ch.title << "\n\n";
        if (!ch.description.empty()) {
            out << "*" << ch.description << "*\n\n";
        }
        for (const auto& pg : ch.pages) {
            out << "### " << pg.title << " [" << pg.signTag << "]\n\n";
            out << pg.content << "\n\n";
            if (!pg.marginalia.empty()) {
                out << "> *Notas al margen:* " << pg.marginalia << "\n\n";
            }
            out << "---\n\n";
        }
    }
    return true;
}

bool BookRepository::Save() {
    std::string path = AppPaths::GetBooksFilePath();
    json root = json::array();

    for (const auto& b : m_Books) {
        json jb;
        jb["id"] = b.id;
        jb["title"] = b.title;
        jb["subtitle"] = b.subtitle;
        jb["author"] = b.author;
        jb["category"] = b.category;
        jb["coverTheme"] = b.coverTheme;
        jb["createdAt"] = b.createdAt;
        jb["updatedAt"] = b.updatedAt;

        json jChapters = json::array();
        for (const auto& ch : b.chapters) {
            json jch;
            jch["id"] = ch.id;
            jch["title"] = ch.title;
            jch["description"] = ch.description;

            json jPages = json::array();
            for (const auto& pg : ch.pages) {
                json jpg;
                jpg["id"] = pg.id;
                jpg["title"] = pg.title;
                jpg["content"] = pg.content;
                jpg["marginalia"] = pg.marginalia;
                jpg["signTag"] = pg.signTag;
                jpg["isBookmarked"] = pg.isBookmarked;
                jpg["createdAt"] = pg.createdAt;
                jpg["updatedAt"] = pg.updatedAt;
                jPages.push_back(jpg);
            }
            jch["pages"] = jPages;
            jChapters.push_back(jch);
        }
        jb["chapters"] = jChapters;
        root.push_back(jb);
    }

    std::ofstream out(path);
    if (!out.is_open()) return false;
    out << root.dump(4);
    m_IsDirty = false;
    return true;
}

bool BookRepository::Load() {
    std::string path = AppPaths::GetBooksFilePath();
    std::ifstream in(path);
    if (!in.is_open()) {
        PopulateDefaultsIfEmpty();
        return false;
    }

    try {
        json root;
        in >> root;
        if (!root.is_array() || root.empty()) {
            PopulateDefaultsIfEmpty();
            return false;
        }

        m_Books.clear();
        for (const auto& jb : root) {
            Book b;
            b.id = jb.value("id", "doc_default");
            b.title = jb.value("title", "Documento");
            b.subtitle = jb.value("subtitle", "");
            b.author = jb.value("author", "Usuario");
            b.category = jb.value("category", "General");
            b.coverTheme = jb.value("coverTheme", "Dark");
            b.createdAt = jb.value("createdAt", "");
            b.updatedAt = jb.value("updatedAt", "");

            if (jb.contains("chapters") && jb["chapters"].is_array()) {
                for (const auto& jch : jb["chapters"]) {
                    Chapter ch;
                    ch.id = jch.value("id", "ch_default");
                    ch.title = jch.value("title", "Sección");
                    ch.description = jch.value("description", "");

                    if (jch.contains("pages") && jch["pages"].is_array()) {
                        for (const auto& jpg : jch["pages"]) {
                            Page pg;
                            pg.id = jpg.value("id", "pg_default");
                            pg.title = jpg.value("title", "Página");
                            pg.content = jpg.value("content", "");
                            pg.marginalia = jpg.value("marginalia", "");
                            pg.signTag = jpg.value("signTag", "GENERAL");
                            pg.isBookmarked = jpg.value("isBookmarked", false);
                            pg.createdAt = jpg.value("createdAt", "");
                            pg.updatedAt = jpg.value("updatedAt", "");
                            ch.pages.push_back(pg);
                        }
                    }
                    if (ch.pages.empty()) {
                        Page p;
                        p.title = "Página 1";
                        p.content = "# " + ch.title + "\n";
                        ch.pages.push_back(p);
                    }
                    b.chapters.push_back(ch);
                }
            }
            if (b.chapters.empty()) {
                Chapter ch;
                ch.title = "Sección Principal";
                Page p;
                p.title = "Página 1";
                p.content = "# " + b.title + "\n";
                ch.pages.push_back(p);
                b.chapters.push_back(ch);
            }
            m_Books.push_back(b);
        }

        SetActiveIndices(0, 0, 0);
        m_IsDirty = false;
        return true;
    } catch (...) {
        PopulateDefaultsIfEmpty();
        return false;
    }
}

void BookRepository::PopulateDefaultsIfEmpty() {
    m_Books.clear();

    // 1. Primary Work & Project Notebook
    {
        Book b1;
        b1.id = "doc_work";
        b1.title = "Work & Project Notebook";
        b1.subtitle = "Planning, meeting notes, and project roadmap";
        b1.author = "User";
        b1.category = "Work";
        b1.coverTheme = "Dark";
        b1.createdAt = GetCurrentTimestampStr();
        b1.updatedAt = b1.createdAt;

        Chapter ch1;
        ch1.id = "ch_overview";
        ch1.title = "Section I: Getting Started";
        ch1.description = "Overview of the workspace structure and formatting options.";

        Page p1;
        p1.id = "p_welcome";
        p1.title = "Welcome to WitcherNotes";
        p1.signTag = "GENERAL";
        p1.isBookmarked = true;
        p1.content =
            "# Welcome to WitcherNotes\n\n"
            "WitcherNotes is a distraction-free digital notebook and document workspace designed for structured knowledge management.\n\n"
            "### Workspace Layout:\n"
            "* **Left Sidebar (Notebook Pages):** Browse all sections and pages. Add new pages, duplicate them, or navigate with a single click.\n"
            "* **Center Pane (Markdown Editor):** Write, format, and edit notes with live syntax styling (headers, bold, lists, quotes, and tables).\n"
            "* **Right Panel (Marginalia & Details):** Keep margin thoughts and quick reminders associated with each page, manage bookmarks, and review metrics.\n\n"
            "---\n\n"
            "> *\"Clarity of thought begins with organized notes.\"*";
        p1.marginalia = "Use this margin panel for quick ideas and task reminders.";
        ch1.pages.push_back(p1);

        Page p2;
        p2.id = "p_shortcuts";
        p2.title = "Keyboard Shortcuts & Tips";
        p2.signTag = "IDEAS";
        p2.isBookmarked = true;
        p2.content =
            "# Essential Shortcuts\n\n"
            "Speed up your workflow with these keyboard commands:\n\n"
            "* **Ctrl + S:** Save all documents and settings immediately.\n"
            "* **Ctrl + N:** Create a new page in the current section.\n"
            "* **Ctrl + B:** Toggle between Library and Workspace views.\n"
            "* **Ctrl + ,:** Open Settings & Preferences.\n"
            "* **Left / Right Arrow:** Navigate sequentially between book pages in reader mode.\n\n"
            "---\n\n"
            "### Markdown Quick Reference:\n"
            "Use the top formatting toolbar or type standard Markdown: `# Heading`, `**bold**`, `*italic*`, `- lists`, and `> quote`.";
        p2.marginalia = "Auto-save runs periodically in the background.";
        ch1.pages.push_back(p2);

        b1.chapters.push_back(ch1);

        Chapter ch2;
        ch2.id = "ch_roadmap";
        ch2.title = "Section II: Project Roadmap";
        ch2.description = "Milestones, tasks, and deliverables.";

        Page p3;
        p3.id = "p_plan";
        p3.title = "Project Milestones";
        p3.signTag = "TASKS";
        p3.isBookmarked = false;
        p3.content =
            "# Project Roadmap\n\n"
            "### Core Objectives:\n"
            "1. Outline initial project specifications and scope.\n"
            "2. Structure modules and technical documentation.\n"
            "3. Draft implementation and review progress regularly.\n"
            "4. Export final deliverables to PDF, EPUB, DOCX, or Markdown.\n\n"
            "### Action Items:\n"
            "* [ ] Review bibliography and source material.\n"
            "* [ ] Bookmark critical reference pages.\n"
            "* [ ] Share document summaries with the team.";
        p3.marginalia = "Review milestone completion on a weekly basis.";
        ch2.pages.push_back(p3);

        b1.chapters.push_back(ch2);
        m_Books.push_back(b1);
    }

    // 2. Personal Research & Knowledge Base
    {
        Book b2;
        b2.id = "doc_research";
        b2.title = "Knowledge Base & Research";
        b2.subtitle = "Curated thoughts, literature highlights, and references";
        b2.author = "User";
        b2.category = "Personal";
        b2.coverTheme = "Paper";
        b2.createdAt = GetCurrentTimestampStr();
        b2.updatedAt = b2.createdAt;

        Chapter ch1;
        ch1.id = "ch_literature";
        ch1.title = "Section I: Curated Insights";
        ch1.description = "Key excerpts and ideas from reading sessions.";

        Page p1;
        p1.id = "p_quote1";
        p1.title = "Literature Notes";
        p1.signTag = "STUDY";
        p1.isBookmarked = true;
        p1.content =
            "# Thoughts & Reflections\n\n"
            "> \"Writing is the painting of the voice.\"\n"
            "> — Voltaire\n\n"
            "Record key insights, inspiring quotations, and foundational concepts for your ongoing projects.";
        p1.marginalia = "Remember to cite original sources and publication dates.";
        ch1.pages.push_back(p1);

        b2.chapters.push_back(ch1);
        m_Books.push_back(b2);
    }

    SetActiveIndices(0, 0, 0);
    m_IsDirty = false;
}

} // namespace WitcherNotes::Core
