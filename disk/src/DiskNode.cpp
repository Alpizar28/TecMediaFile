#include "DiskNode.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

DiskNode& DiskNode::instance() {
    static DiskNode inst;
    return inst;
}

bool DiskNode::writeBlock(int id, const std::string& data_b64) {
    fs::create_directories("blocks");
    std::string path = "blocks/block_" + std::to_string(id) + ".dat";
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::cerr << "Error escribiendo bloque " << id << "\n";
        return false;
    }
    // En producción: decodifica base64. Aquí almacenamos raw.
    out << data_b64;
    return true;
}

std::string DiskNode::readBlock(int id) {
    std::string path = "blocks/block_" + std::to_string(id) + ".dat";
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cerr << "Error leyendo bloque " << id << "\n";
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
}
