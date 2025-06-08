#include <tinyxml2.h>
#include <iostream>

int main() {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile("controller/config/controller_config.xml") != tinyxml2::XML_SUCCESS) {
        std::cerr << "No se pudo leer el XML.\n";
        return 1;
    }

    auto* root = doc.FirstChildElement("controller");
    if (!root) {
        std::cerr << "No se encontró <controller>\n";
        return 1;
    }

    int port = 0;
    root->FirstChildElement("port")->QueryIntText(&port);

    std::cout << "Puerto leído desde XML: " << port << "\n";
    return 0;
}
