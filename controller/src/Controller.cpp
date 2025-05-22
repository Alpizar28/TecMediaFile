#include "Controller.h"

// Instancia singleton
Controller& Controller::instance() {
    static Controller inst;
    return inst;
}

bool Controller::addDocument(const std::string& name, const std::string& data_b64) {
    // Aquí iría tu lógica: dividir en bloques, llamar a disknodes…
    // Por ahora devolvemos true a modo de stub.
    return true;
}

std::vector<std::string> Controller::listDocuments() {
    // Stub de ejemplo
    return { "doc1", "doc2" };
}

std::string Controller::downloadDocument(const std::string& name) {
    // Stub: en base64 o datos reales
    return "PDF_EN_BASE64...";
}
