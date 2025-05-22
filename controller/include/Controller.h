#pragma once
#include <string>
#include <vector>

class Controller {
public:
    static Controller& instance();

    // Operaciones REST
    bool addDocument(const std::string& name, const std::string& data_b64);
    std::vector<std::string> listDocuments();
    std::string downloadDocument(const std::string& name);

private:
    Controller() = default;
    // atributos privados: lista de nombres, config, etc.
};
