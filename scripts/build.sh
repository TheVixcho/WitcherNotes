#!/usr/bin/env bash
set -e
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"
ACTION="${1:-linux}"

export PATH="/home/vixcho/.local/bin:$PATH"

case "$ACTION" in
    linux)
        echo "=== [WitcherNotes] Configurando y Compilando para Linux ==="
        cmake -B build-linux -G Ninja -DCMAKE_BUILD_TYPE=Release
        cmake --build build-linux
        echo "=== [WitcherNotes] Compilación finalizada con éxito ==="
        ;;
    linux-run)
        if [ ! -f "build-linux/WitcherNotes" ]; then
            echo "=== [WitcherNotes] Binario no encontrado. Compilando primero... ==="
            cmake -B build-linux -G Ninja -DCMAKE_BUILD_TYPE=Release
            cmake --build build-linux
        fi
        echo "=== [WitcherNotes] Ejecutando WitcherNotes... ==="
        cd build-linux
        ./WitcherNotes
        ;;
    clean)
        echo "=== [WitcherNotes] Limpiando carpetas de compilación... ==="
        rm -rf build-linux
        echo "=== Limpieza completada ==="
        ;;
    *)
        echo "Uso: $0 [linux | linux-run | clean]"
        exit 1
        ;;
esac
