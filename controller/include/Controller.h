// controller/include/Controller.h
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

class Controller {
public:
    static Controller& instance();

    // Lee el PDF samples/<name>.pdf, lo codifica en Base64 y lo guarda
    bool addDocument(const std::string& name, const std::string& /*data_b64*/ = "");

    // Lista todos los documentos añadidos
    std::vector<std::string> listDocuments();

    // Devuelve el Base64 completo del PDF
    std::string downloadDocument(const std::string& name);

private:
    Controller() = default;

    std::vector<std::string> docs_;  
    std::unordered_map<std::string, std::string> data_;  
};
