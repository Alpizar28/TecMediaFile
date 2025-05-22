#!/usr/bin/env bash
set -euo pipefail

# -------------------------------
# Script para construir y ejecutar
# Controller y DiskNode de Crow
# -------------------------------

# 1. Variables
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

# 2. Limpiar y crear build
echo "🛠  Limpiando y creando carpeta de compilación..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# 3. Generar Makefiles con CMake
echo "📐  Ejecutando cmake..."
cmake -S "$ROOT_DIR" -B "$BUILD_DIR"

# 4. Compilar todo
echo "🚧  Compilando proyecto..."
cmake --build "$BUILD_DIR" -- -j"$(nproc)"

# 5. Ejecutar Controller y DiskNode
echo "▶️  Iniciando Controller y DiskNode..."
# Controller en background
"$BUILD_DIR/controller/controller" &
PID_CTRL=$!
# DiskNode en background
"$BUILD_DIR/disk/disknode" &
PID_DISK=$!

echo "   Controller PID: $PID_CTRL"
echo "   DiskNode   PID: $PID_DISK"
echo
echo "🌐  Controller disponible en http://localhost:18080/hello"
echo
echo "💡  Para detenerlos: kill $PID_CTRL $PID_DISK"

# 6. Esperar a que terminen (opcional)
wait
