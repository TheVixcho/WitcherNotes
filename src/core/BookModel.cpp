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
    if (upper == "IDEAS" || upper == "AARD")         return NoteCategory::Ideas;
    if (upper == "DRAFT" || upper == "BORRADOR")      return NoteCategory::Borrador;
    if (upper == "IMPORTANT" || upper == "IMPORTANTE" || upper == "IGNI") return NoteCategory::Importante;
    if (upper == "REFERENCE" || upper == "REFERENCIA" || upper == "BESTIARY") return NoteCategory::Referencia;
    if (upper == "TASKS" || upper == "TAREAS" || upper == "YRDEN")   return NoteCategory::Tareas;
    if (upper == "PROJECT" || upper == "PROYECTO" || upper == "QUEST") return NoteCategory::Proyecto;
    if (upper == "STUDY" || upper == "ESTUDIO" || upper == "ALCHEMY" || upper == "LORE") return NoteCategory::Estudio;
    return NoteCategory::General;
}

const char* CategoryDisplayName(NoteCategory cat) {
    switch (cat) {
        case NoteCategory::Ideas:       return "Ideas y Bocetos";
        case NoteCategory::Borrador:    return "Borrador";
        case NoteCategory::Importante:  return "Prioritario / Importante";
        case NoteCategory::Referencia:  return "Referencia y Recursos";
        case NoteCategory::Tareas:      return "Tareas y Acciones";
        case NoteCategory::Proyecto:    return "Proyecto";
        case NoteCategory::Estudio:     return "Estudio e Investigación";
        default:                        return "Nota General";
    }
}

const char* CategorySymbol(NoteCategory cat) {
    switch (cat) {
        case NoteCategory::Ideas:       return "[💡 IDEAS]";
        case NoteCategory::Borrador:    return "[✏ BORRADOR]";
        case NoteCategory::Importante:  return "[⭐ IMPORTANTE]";
        case NoteCategory::Referencia:  return "[📚 REFERENCIA]";
        case NoteCategory::Tareas:      return "[☑ TAREAS]";
        case NoteCategory::Proyecto:    return "[🚀 PROYECTO]";
        case NoteCategory::Estudio:     return "[🔬 ESTUDIO]";
        default:                        return "[📝 NOTA]";
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
    b.title = title.empty() ? "Nuevo Documento" : title;
    b.subtitle = subtitle;
    b.author = author.empty() ? "Usuario" : author;
    b.category = "Documentos";
    b.coverTheme = coverTheme.empty() ? "Dark" : coverTheme;
    b.createdAt = GetCurrentTimestampStr();
    b.updatedAt = b.createdAt;

    Chapter ch;
    ch.id = "ch_1";
    ch.title = "Sección 1";
    ch.description = "Primera sección del documento";

    Page p;
    p.id = "pg_1";
    p.title = "Página 1";
    p.content = "# " + b.title + "\n\nComienza a escribir aquí tu documento o notas...\n";
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
    ch.title = title.empty() ? ("Sección " + std::to_string(b->chapters.size() + 1)) : title;
    ch.description = "";

    Page p;
    p.id = "pg_" + std::to_string(std::time(nullptr));
    p.title = "Nueva Hoja";
    p.content = "# " + ch.title + "\n\nEscribe aquí tus apuntes...\n";
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
    p.title = title.empty() ? ("Hoja " + std::to_string(ch->pages.size() + 1)) : title;
    p.content = "## " + p.title + "\n\nEscribe aquí tus notas...\n";
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

    // 1. Cuaderno Principal de Proyectos y Notas
    {
        Book b1;
        b1.id = "doc_proyectos";
        b1.title = "Cuaderno de Trabajo y Proyectos";
        b1.subtitle = "Planificación, notas de reuniones y apuntes diarios";
        b1.author = "Usuario";
        b1.category = "Trabajo";
        b1.coverTheme = "Dark";
        b1.createdAt = GetCurrentTimestampStr();
        b1.updatedAt = b1.createdAt;

        Chapter ch1;
        ch1.id = "ch_guia";
        ch1.title = "Sección I: Guía y Organización";
        ch1.description = "Introducción a la estructura organizada del cuaderno.";

        Page p1;
        p1.id = "p_bienvenida";
        p1.title = "Bienvenido a tu Cuaderno de Notas";
        p1.signTag = "GENERAL";
        p1.isBookmarked = true;
        p1.content =
            "# Bienvenido a tu Cuaderno de Notas\n\n"
            "Esta aplicación te permite organizar tus documentos y cuadernos de forma clara, rápida y sin distracciones.\n\n"
            "### Cómo está organizada la pantalla:\n"
            "* **Columna Izquierda (Hojas del Documento):** Aquí ves todas las secciones y páginas del documento. Puedes añadir nuevas hojas, duplicarlas o navegar entre ellas con un solo clic.\n"
            "* **Columna Central (Editor del Documento):** Tu espacio principal para escribir y formatear tus textos con Markdown (encabezados, negrita, listas, citas, etc.).\n"
            "* **Columna Derecha (Notas, Marcadores e Info):** Espacio para ir dejando notas al margen y apuntes rápidos asociados a cada página, gestionar marcadores favoritos y ver estadísticas del documento.\n\n"
            "---\n\n"
            "> *\"Organiza tus ideas con claridad y escribe con fluidez.\"*";
        p1.marginalia = "Puedes usar este panel derecho para dejar ideas rápidas y recordatorios.";
        ch1.pages.push_back(p1);

        Page p2;
        p2.id = "p_atajos";
        p2.title = "Atajos de Teclado y Consejos";
        p2.signTag = "IDEAS";
        p2.isBookmarked = true;
        p2.content =
            "# Atajos Útiles\n\n"
            "Optimiza tu flujo de trabajo con estos comandos rápidos:\n\n"
            "* **Ctrl + S:** Guarda inmediatamente todos tus cambios y configuraciones.\n"
            "* **Ctrl + N:** Crea una nueva hoja de notas al instante.\n"
            "* **Flechas Izquierda / Derecha:** Navega secuencialmente entre páginas del documento.\n"
            "* **Botón Volver a la Biblioteca:** Regresa a la estantería de documentos en cualquier momento.\n\n"
            "---\n\n"
            "### Formato Markdown:\n"
            "Usa la barra de herramientas superior o escribe directamente `# Título`, `**negrita**`, `*cursiva*`, `- listas` y `1. tareas`.";
        p2.marginalia = "El autoguardado está activo por defecto cada minuto.";
        ch1.pages.push_back(p2);

        b1.chapters.push_back(ch1);

        Chapter ch2;
        ch2.id = "ch_proyectos";
        ch2.title = "Sección II: Notas de Proyectos";
        ch2.description = "Espacio para tareas, objetivos e hitos.";

        Page p3;
        p3.id = "p_plan";
        p3.title = "Plan de Trabajo y Objetivos";
        p3.signTag = "TASKS";
        p3.isBookmarked = false;
        p3.content =
            "# Plan de Trabajo\n\n"
            "### Objetivos Principales:\n"
            "1. Definir los requerimientos iniciales del proyecto.\n"
            "2. Estructurar los capítulos y módulos de trabajo.\n"
            "3. Redactar los borradores y documentación clave.\n"
            "4. Revisión final y exportación a Markdown.\n\n"
            "### Tareas Pendientes:\n"
            "* [ ] Revisar fuentes y referencias bibliográficas.\n"
            "* [ ] Añadir marcadores a las páginas críticas.\n"
            "* [ ] Compartir resumen con el equipo.";
        p3.marginalia = "Hacer seguimiento semanal de los hitos completados.";
        ch2.pages.push_back(p3);

        b1.chapters.push_back(ch2);
        m_Books.push_back(b1);
    }

    // 2. Cuaderno Personal de Ideas y Referencias
    {
        Book b2;
        b2.id = "doc_ideas";
        b2.title = "Cuaderno Personal de Ideas y Lecturas";
        b2.subtitle = "Apuntes libres, resúmenes y referencias bibliográficas";
        b2.author = "Usuario";
        b2.category = "Personal";
        b2.coverTheme = "Paper";
        b2.createdAt = GetCurrentTimestampStr();
        b2.updatedAt = b2.createdAt;

        Chapter ch1;
        ch1.id = "ch_lecturas";
        ch1.title = "Sección I: Citas y Resúmenes";
        ch1.description = "Fragmentos interesantes extraídos de lecturas.";

        Page p1;
        p1.id = "p_resumen1";
        p1.title = "Apuntes de Lectura";
        p1.signTag = "STUDY";
        p1.isBookmarked = true;
        p1.content =
            "# Citas y Reflexiones\n\n"
            "> \"Escribir es la pintura de la voz.\"\n"
            "> — Voltaire\n\n"
            "Anota aquí pasajes destacados, ideas inspiradoras y conceptos que desees conservar para tus proyectos.";
        p1.marginalia = "Recordar citar siempre la fuente original.";
        ch1.pages.push_back(p1);

        b2.chapters.push_back(ch1);
        m_Books.push_back(b2);
    }

    SetActiveIndices(0, 0, 0);
    m_IsDirty = false;
}

} // namespace WitcherNotes::Core
