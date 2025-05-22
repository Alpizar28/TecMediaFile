#pragma once
#include <string>

class DiskNode {
public:
    static DiskNode& instance();

    // Escribe el bloque 'id' (base64) en un fichero
    bool writeBlock(int id, const std::string& data_b64);

    // Lee el bloque 'id' desde fichero (base64)
    std::string readBlock(int id);

private:
    DiskNode() = default;
    // Aquí podrías guardar configuración, ruta de blocks/, etc.
};
