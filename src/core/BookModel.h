#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace WitcherNotes::Core {

enum class NoteCategory {
    General = 0,
    Ideas,
    Borrador,
    Importante,
    Referencia,
    Tareas,
    Proyecto,
    Estudio
};

std::string CategoryToString(NoteCategory cat);
NoteCategory StringToCategory(const std::string& str);
const char* CategoryDisplayName(NoteCategory cat);
const char* CategorySymbol(NoteCategory cat);

// Alias de compatibilidad
using WitcherSign = NoteCategory;
inline std::string SignToString(WitcherSign sign) { return CategoryToString(sign); }
inline WitcherSign StringToSign(const std::string& str) { return StringToCategory(str); }
inline const char* SignDisplayName(WitcherSign sign) { return CategoryDisplayName(sign); }
inline const char* SignRuneSymbol(WitcherSign sign) { return CategorySymbol(sign); }

struct Page {
    std::string id;
    std::string title;
    std::string content;
    std::string marginalia; // Notas al margen / apuntes rápidos
    std::string signTag = "GENERAL"; // Categoría o etiqueta
    bool isBookmarked = false;
    std::string createdAt;
    std::string updatedAt;

    int GetWordCount() const;
    int GetReadingTimeMinutes() const;
};

struct Chapter {
    std::string id;
    std::string title;
    std::string description;
    std::vector<Page> pages;

    int GetTotalWords() const;
};

struct Book {
    std::string id;
    std::string title;
    std::string subtitle;
    std::string author;
    std::string category;
    std::string coverTheme; // "Dark", "Paper", "Purple", "Sapphire", "Warm"
    std::vector<Chapter> chapters;
    std::string createdAt;
    std::string updatedAt;

    int GetTotalPages() const;
    int GetTotalWords() const;
    int GetGlobalPageIndex(int chapterIdx, int pageIdx) const;
    bool GetChapterAndPageFromGlobalIndex(int globalIdx, int& outChapterIdx, int& outPageIdx) const;
};

class BookRepository {
public:
    static BookRepository& Get();

    bool Load();
    bool Save();
    void PopulateDefaultsIfEmpty();

    std::vector<Book>& GetBooks() { return m_Books; }
    const std::vector<Book>& GetBooks() const { return m_Books; }

    int GetActiveBookIndex() const { return m_ActiveBookIdx; }
    int GetActiveChapterIndex() const { return m_ActiveChapterIdx; }
    int GetActivePageIndex() const { return m_ActivePageIdx; }

    void SetActiveIndices(int bookIdx, int chapterIdx, int pageIdx);
    void SetActivePage(int pageIdx);
    void SetActiveChapter(int chapterIdx);
    void SetActiveBook(int bookIdx);

    Book* GetActiveBook();
    Chapter* GetActiveChapter();
    Page* GetActivePage();

    // Navegación de páginas
    bool HasNextPage() const;
    bool HasPrevPage() const;
    void NextPage();
    void PrevPage();

    // Operaciones CRUD
    void AddBook(const std::string& title, const std::string& subtitle, const std::string& author, const std::string& coverTheme);
    void DeleteBook(int index);
    void AddChapter(const std::string& title);
    void DeleteChapter(int chapterIdx);
    void AddPage(const std::string& title, const std::string& signTag = "GENERAL");
    void DeletePage(int pageIdx);
    void DuplicateCurrentPage();

    // Búsqueda en todos los documentos
    struct SearchResult {
        int bookIdx;
        int chapterIdx;
        int pageIdx;
        std::string bookTitle;
        std::string chapterTitle;
        std::string pageTitle;
        std::string matchSnippet;
    };
    std::vector<SearchResult> Search(const std::string& query) const;

    // Exportación
    bool ExportActivePageToMarkdown(const std::string& filepath);
    bool ExportActiveBookToMarkdown(const std::string& filepath);

    bool IsDirty() const { return m_IsDirty; }
    void MarkDirty() { m_IsDirty = true; }

private:
    BookRepository();
    std::vector<Book> m_Books;
    int m_ActiveBookIdx = 0;
    int m_ActiveChapterIdx = 0;
    int m_ActivePageIdx = 0;
    bool m_IsDirty = false;
};

} // namespace WitcherNotes::Core
