#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$ROOT/build"

echo "🛠 Limpiando build..."
rm -rf "$BUILD"
mkdir -p "$BUILD"

echo "📐 Ejecutando cmake..."
cmake -S "$ROOT" -B "$BUILD"

echo "🚧 Compilando..."
cmake --build "$BUILD" --target controller -- -j$(nproc)
cmake --build "$BUILD" --target disknode  -- -j$(nproc)

echo "▶️ Arrancando servicios..."

# Arranca Controller desde la raíz (cwd=ROOT) para leer samples/
(
  cd "$ROOT"
  echo "  Iniciando Controller..."
  "$BUILD/controller/controller" &
  PID_CTRL=$!
)

# Arranca DiskNode con cwd en BUILD
# Arranca DiskNode con cwd en ROOT para que lea bien disk/config
(
  cd "$ROOT"
  echo "  Iniciando DiskNode..."
  "$BUILD/disk/disknode" &
  PID_DISK=$!
)


echo
echo "Controller PID=$PID_CTRL (http://localhost:18080)"
echo "DiskNode   PID=$PID_DISK (http://localhost:18081)"
echo
echo "Para detenerlos: kill $PID_CTRL $PID_DISK"

wait
