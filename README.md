# WitcherNotes

A native, high-performance Markdown notebook and document workspace built in **C++17** and **Dear ImGui (Docking)** with **OpenGL 3** hardware acceleration.

Licensed under the permissive [MIT License](LICENSE). WitcherNotes is fully open-source, offline-first, and private by design, with no subscriptions, cloud lock-in, or mandatory accounts.

---

## Key Features

### 1. Document Reading & Viewing Experience
* **Two-Page Book Spread:** Dual-page layout with center spine shadow, custom margins, and book geometry.
* **Single-Page View:** Centered reader layout with balanced column width for long-form reading and editing.
* **Continuous Scroll Mode:** Fluid vertical reading stream with distinct section dividers.
* **Navigation:** Quick page navigation with arrow keys, Page Up/Down, or Spacebar.
* **Custom Typography:** Real-time font size adjustments and page margin customization.
* **Bookmarks & Marginalia:** Save quick access to important pages and add margin notes linked directly to individual documents.

### 2. Multi-Format Export Engine
Export single pages or entire notebooks into multiple publishing standards:
* **PDF Document (.pdf):** Clean editorial layout.
* **Digital Book (.epub):** Compatible with e-readers and mobile devices.
* **Microsoft Word (.docx):** Structured document format preserving headers, lists, and quotes.
* **Markdown (.md):** Clean plain text Markdown files for portable note-taking.

### 3. Local-First Architecture & Privacy
* **Local Storage:** All notebooks and pages are stored locally on your machine in standard JSON format.
* **Offline-First:** Fully functional without an internet connection.
* **Auto-Update Engine:** Built-in update checker and installer for GitHub releases (both public and private repositories via personal access tokens).

---

## Project Structure

```text
WitcherNotes/
|-- LICENSE                     # MIT Open Source License
|-- CMakeLists.txt              # CMake build configuration
|-- scripts/
|   |-- build.sh                # Linux build script
|   `-- exporter.py             # Export engine for EPUB, DOCX, and PDF
|-- assets/
|   |-- fonts/                  # TTF fonts (RobotoSlab, OpenSans, etc.)
|   `-- images/                 # App logos and splash assets
|-- src/
|   |-- main.cpp                # Application entrypoint, GLFW window, DPI handling
|   |-- SplashScreen.h/.cpp     # Animated multi-stage startup loader
|   |-- Version.h               # Version metadata and constants
|   |-- core/
|   |   |-- AppPaths.h/.cpp     # Standard platform path resolution
|   |   |-- BookModel.h/.cpp    # Document, chapter, and page data models
|   |   |-- ExportManager.h/.cpp# Export dispatcher
|   |   |-- SettingsManager.h/.cpp # Theme palettes and reader preferences
|   |   `-- UpdateManager.h/.cpp# GitHub release update checking & installation
|   `-- ui/
|       |-- UIManager.h/.cpp    # Master UI coordinator, dockspace, and menus
|       `-- panels/
|           |-- EditorPanel.h/.cpp       # Markdown editor with formatting toolbar
|           |-- LibraryPanel.h/.cpp      # Notebook gallery and global search
|           |-- ReaderPanel.h/.cpp       # Two-page spread & continuous reader
|           |-- SettingsPanel.h/.cpp     # Application preferences and updates
|           `-- TocBookmarksPanel.h/.cpp # Table of contents, bookmarks, and marginalia
|-- vendor/
|   |-- nlohmann/               # JSON serialization library
|   `-- stb/                    # STB image library
`-- external/
    `-- imgui-docking/          # Dear ImGui docking branch
```

---

## Keyboard Shortcuts

| Shortcut | Description |
|---|---|
| **Left Arrow** | Previous page in reader mode |
| **Right Arrow** | Next page in reader mode |
| **Space / PageUp / PageDown** | Quick page navigation |
| **Ctrl + S** | Save all notebooks and preferences |
| **Ctrl + N** | Add a new page to current section |
| **Ctrl + B** | Toggle between Library and Workspace |
| **Ctrl + ,** | Open Settings and Preferences |
| **F11** | Toggle fullscreen mode |

---

## Building from Source

### Prerequisites (Linux)
* CMake 3.16+
* C++17 compatible compiler (GCC 9+ or Clang 10+)
* Ninja or GNU Make
* OpenGL, GLFW3, GLEW, and X11 development headers

On Debian / Ubuntu:
```bash
sudo apt-get install build-essential cmake ninja-build libglfw3-dev libglew-dev libx11-dev libcurl4-openssl-dev
```

### Build Instructions
```bash
./scripts/build.sh linux
```

### Run
```bash
./scripts/build.sh linux-run
```
or launch the binary directly:
```bash
./build-linux/WitcherNotes
```

---

## Packaging

To generate a standalone AppImage:
```bash
./scripts/package-appimage.sh
```

---

## License

Distributed under the **MIT License**. See [`LICENSE`](LICENSE) for details.
