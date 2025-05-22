// controller/src/Controller.cpp
#include "Controller.h"
#include "../../shared/include/Base64.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

Controller& Controller::instance() {
    static Controller inst;
    return inst;
}

bool Controller::addDocument(const std::string& name, const std::string&) {
    // Ruta al PDF de muestra
    std::string path = "samples/" + name + ".pdf";
    if (!fs::exists(path)) {
        std::cerr << "Controller: no existe " << path << "\n";
        return false;
    }
    // Leer fichero binario
    std::ifstream in(path, std::ios::binary);
    std::ostringstream buf;
    buf << in.rdbuf();
    std::string bytes = buf.str();

    // Codificar a Base64
    std::string b64 = base64_encode(bytes);

    // Almacenar
    docs_.push_back(name);
    data_[name] = std::move(b64);
    return true;
}

std::vector<std::string> Controller::listDocuments() {
    return docs_;
}

std::string Controller::downloadDocument(const std::string& name) {
    auto it = data_.find(name);
    if (it == data_.end()) {
        std::cerr << "Controller: downloadDocument, nombre no encontrado: " << name << "\n";
        return "";
    }
    return it->second;
}
