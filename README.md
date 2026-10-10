# WitcherNotes — Grimorio Digital & E-Book Note Engine

> **Software 100% Libre y de Código Abierto (Open Source)** bajo Licencia [MIT](LICENSE).  
> Sin planes de pago, sin suscripciones y sin necesidad de crear cuentas.

**WitcherNotes** es una aplicación nativa de notas y gestión de conocimiento desarrollada en **C++17** y **Dear ImGui (Docking)** con aceleración gráfica **OpenGL 3**, diseñada con la estética inmersiva de un **Grimorio / Libro Digital (E-Book)**.

Basada en una sólida arquitectura modular (SplashScreen con animación por pasos, gestión de DPI, paletas de diseño desacopladas, persistencia JSON local y DockSpace avanzado).

---

## Caracteristicas Principales

### 1. Experiencia Tipo E-Book (Lector Digital)
* **Modo Doble Página (Libro Abierto / Hardcover Tome):** Visualiza tus notas como un libro físico con pliegue de encuadernación central con sombras, números de página, encabezados elegantes y márgenes configurables.
* **Modo Página Única (E-Reader Centrado):** Enfoque clásico de lectura con ancho de columna optimizado para no cansar la vista.
* **Modo Pergamino Continuo (Scroll):** Lectura vertical sin interrupciones con separadores ornamentales.
* **Navegación Fluida:** Pasa páginas con `Flecha Izquierda / A` y `Flecha Derecha / D`, o barra espaciadora.
* **Control Tipográfico:** Modifica el tamaño de fuente y márgenes en tiempo real con fuentes Serif de alta legibilidad (`RobotoSlab`, `OpenSans`).
* **Marcadores (Bookmarks):** Marca tus páginas predilectas para saltar a ellas rápidamente.
* **Anotaciones al Margen (Marginalia):** Añade glosas y pensamientos laterales en el margen del pergamino.

### 2. Exportación Multiformato Completa (Libre y Gratuita)
* **Documentos PDF:** Genera documentos con maquetación editorial.
* **Libros Digitales EPUB (.epub):** Compatible con e-readers y lectores de libros digitales.
* **Microsoft Word (.docx):** Documentos editables estructurados por capítulos y hojas.
* **Markdown (.md):** Exportación limpia y directa sin dependencias propietarias.

### 3. Privacidad y Funcionamiento 100% Offline
* **100% Local:** Tus notas y cuadernos se guardan en tu equipo de forma privada y segura.
* **Sin Registro Forzado:** No necesitas registrarte ni crear cuentas para disfrutar de todas las funciones.
* **Sin Dependencias de Nube:** Todo el funcionamiento es autónomo en tu máquina.

---

## Estructura del Codigo

```text
WitcherNotes/
├── LICENSE                     # Licencia de código abierto MIT
├── CMakeLists.txt              # Configuración de compilación con Ninja / Make
├── scripts/
│   └── build.sh               # Script de compilación y ejecución automática
├── assets/
│   ├── fonts/                 # RobotoSlab, OpenSans, NotoEmoji, Symbola
│   └── images/                # witchernotes_logo.png, fondos de splash
├── src/
│   ├── main.cpp               # Punto de entrada, DPI, ciclo de render y GLFW
│   ├── SplashScreen.h/.cpp    # SplashScreen modular por pasos y transición
│   ├── Version.h              # Metadatos de versión y compilación
│   ├── core/
│   │   ├── AppPaths.h/.cpp    # Rutas del sistema (~/.config/WitcherNotes)
│   │   ├── BookModel.h/.cpp   # Modelo Libro, Capítulo, Página y persistencia
│   │   ├── ExportManager.h/.cpp # Motores de exportación (PDF, EPUB, DOCX, MD)
│   │   ├── UpdateManager.h/.cpp # Verificación y descarga de actualizaciones
│   │   └── SettingsManager.h/.cpp # Gestión de temas y configuración de lectura
│   ├── ui/
│   │   ├── UIManager.h/.cpp   # Coordinador principal, DockSpace y menús
│   │   └── panels/
│   │       ├── ReaderPanel.h/.cpp       # Lector E-Book (Doble página / Spread)
│   │       ├── EditorPanel.h/.cpp       # Editor con barra de formato y marginalia
│   │       ├── LibraryPanel.h/.cpp      # Estantería de grimorios y búsqueda
│   │       ├── TocBookmarksPanel.h/.cpp # Índice interactivo y marcadores
│   │       └── SettingsPanel.h/.cpp     # Ajustes del lector y temas
│   ├── vendor/
│   │   ├── nlohmann/          # json.hpp para serialización
│   │   └── stb/               # stb_image para texturas
│   └── external/
│       └── imgui-docking/     # Dear ImGui con soporte de acoplamiento de paneles
```

---

## Atajos de Teclado

| Atajo | Acción |
|---|---|
| **Flecha Izquierda / A** | Página anterior del E-Book |
| **Flecha Derecha / D** | Página siguiente del E-Book |
| **Espacio / RePág / AvPág** | Navegación rápida entre páginas |
| **Ctrl + S** | Guardar todos los grimorios y ajustes |
| **Ctrl + N** | Crear una nueva página en el capítulo actual |
| **Ctrl + B** | Alternar entre Biblioteca y Espacio de Trabajo |
| **Ctrl + ,** | Abrir Ajustes y Preferencias |
| **F11** | Alternar modo inmersivo |

---

## Como Compilar y Ejecutar

Desde la carpeta `WitcherNotes`:

### Compilar:
```bash
./scripts/build.sh linux
```

### Ejecutar directamente:
```bash
./scripts/build.sh linux-run
```
o directamente el binario generado:
```bash
./build-linux/WitcherNotes
```

---

## Licencia

Distribuido bajo la **Licencia MIT**. Consulta el archivo [`LICENSE`](LICENSE) para más detalles.
