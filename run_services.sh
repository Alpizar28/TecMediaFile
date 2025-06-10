#!/usr/bin/env bash
set -euo pipefail

# Ruta del proyecto
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$ROOT/build"

# Liberar puertos en caso de instancias previas
echo "🚿 Liberando puertos 18080 y 18081..."
# fuser cierra procesos que escuchan en esos puertos
fuser -k 18080/tcp || true
fuser -k 18081/tcp || true

# Limpiar y crear build
echo "🛠 Limpiando build..."
rm -rf "$BUILD"
mkdir -p "$BUILD"

# Configurar con CMake
echo "📐 Ejecutando cmake..."
cmake -S "$ROOT" -B "$BUILD"

# Compilar targets
echo "🚧 Compilando..."
cmake --build "$BUILD" --target controller -- -j"$(nproc)"
cmake --build "$BUILD" --target disknode  -- -j"$(nproc)"

# Arrancar servicios
echo "▶️ Arrancando servicios..."

# Controller (cwd=ROOT para leer samples/)
pushd "$ROOT" > /dev/null
echo "  Iniciando Controller..."
"$BUILD/controller/controller" &
PID_CTRL=$!
popd > /dev/null

# DiskNode (cwd=ROOT para leer disk/config/...)
pushd "$ROOT" > /dev/null
echo "  Iniciando DiskNode..."
"$BUILD/disk/disknode" &
PID_DISK=$!
popd > /dev/null

# Mensaje final con PIDs y URLs
echo
echo "Controller PID=$PID_CTRL (http://localhost:18080)"
echo "DiskNode   PID=$PID_DISK (http://localhost:18081)"
echo

echo "Para detenerlos: kill $PID_CTRL $PID_DISK"

# Esperar a que terminen los procesos
wait
