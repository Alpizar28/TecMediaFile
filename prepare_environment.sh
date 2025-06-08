#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "🗂 Creando carpetas necesarias..."
mkdir -p "$ROOT/controller/config"
mkdir -p "$ROOT/disk/config"
mkdir -p "$ROOT/disk/blocks"

echo "📝 Generando controller_config.xml..."
cat > "$ROOT/controller/config/controller_config.xml" <<EOF
<controller>
    <port>18080</port>
    <disks>
        <disk>
            <ip>127.0.0.1</ip>
            <port>18081</port>
        </disk>
        <disk>
            <ip>127.0.0.1</ip>
            <port>18082</port>
        </disk>
        <disk>
            <ip>127.0.0.1</ip>
            <port>18083</port>
        </disk>
        <disk>
            <ip>127.0.0.1</ip>
            <port>18084</port>
        </disk>
    </disks>
</controller>
EOF

echo "📝 Generando disk_config.xml..."
cat > "$ROOT/disk/config/disk_config.xml" <<EOF
<disk>
    <ip>127.0.0.1</ip>
    <port>18081</port>
    <path>blocks/</path>
    <size>1048576</size>
    <block_size>4096</block_size>
</disk>
EOF

echo "✅ Entorno preparado. Ya puedes correr ./run_services.sh"
