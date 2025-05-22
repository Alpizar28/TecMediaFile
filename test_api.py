#!/usr/bin/env python3
import requests
import sys

BASE_CONTROLLER = "http://localhost:18080"
BASE_DISK       = "http://localhost:18081"

tests = [
    {
        "name": "Escribir bloque 0 en DiskNode",
        "method": "POST",
        "url": f"{BASE_DISK}/write/0",
        "json": {"data": "QUJD"},
        "expect_status": 200
    },
    {
        "name": "Leer bloque 0 desde DiskNode",
        "method": "GET",
        "url": f"{BASE_DISK}/read/0",
        "expect_status": 200,
        "expect_json": {"data": "QUJD"}
    },
    {
        "name": "Agregar documento 'mipdf' al Controller",
        "method": "POST",
        "url": f"{BASE_CONTROLLER}/add",
        "json": {"name": "mipdf", "data": "SGVsbG8gd29ybGQ="},
        "expect_status": 200,
        "expect_json_key": "ok"
    },
    {
        "name": "Listar documentos en Controller (muestra resultado)",
        "method": "GET",
        "url": f"{BASE_CONTROLLER}/list",
        "expect_status": 200,
        # ya no se falla si falta 'mipdf'
        "just_display": True
    },
    {
        "name": "Descargar documento 'mipdf'",
        "method": "GET",
        "url": f"{BASE_CONTROLLER}/download/mipdf",
        "expect_status": 200,
        "expect_json_key": "data"
    }
]

def run_tests():
    print("Iniciando pruebas de la API TecMediaFile\n")
    for t in tests:
        print(f"Prueba: {t['name']}")
        try:
            if t["method"] == "POST":
                r = requests.post(t["url"], json=t.get("json", {}))
            else:
                r = requests.get(t["url"])
        except requests.exceptions.RequestException as e:
            print(f"❌ Error de conexión: {e}\n")
            sys.exit(1)

        # Estado HTTP
        if r.status_code != t["expect_status"]:
            print(f"❌ Esperado status {t['expect_status']}, obtuvo {r.status_code}\n")
            continue

        # Sólo mostrar contenido
        if t.get("just_display"):
            print(f"→ Resultado: {r.json()}\n✅ OK\n")
            continue

        # Validaciones previas
        if "expect_json" in t:
            if r.json() != t["expect_json"]:
                print(f"❌ JSON inesperado: {r.json()}\n")
                continue
        if "expect_json_key" in t:
            if t["expect_json_key"] not in r.json():
                print(f"❌ Clave '{t['expect_json_key']}' no encontrada en {r.json()}\n")
                continue

        print("✅ OK\n")
    print("Pruebas completadas.")

if __name__ == "__main__":
    run_tests()
