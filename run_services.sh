#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$ROOT/build"

echo "🛠 Limpiando build..."
rm -rf "$BUILD"
mkdir -p "$BUILD"
cd "$BUILD"

echo "📐 Ejecutando cmake..."
cmake -S "$ROOT" -B "$BUILD"

echo "🚧 Compilando..."
cmake --build . --target controller -- -j$(nproc)
cmake --build . --target disknode  -- -j$(nproc)

echo "▶️ Arrancando Controller (18080) y DiskNode (18081)..."
"$BUILD/controller/controller" &
PID1=$!
"$BUILD/disk/disknode" &
PID2=$!

echo "Controller PID=$PID1  DiskNode PID=$PID2"
echo "→ http://localhost:18080/list"
echo "→ http://localhost:18081/read/0"
echo
echo "Para detenerlos: kill $PID1 $PID2"

wait
