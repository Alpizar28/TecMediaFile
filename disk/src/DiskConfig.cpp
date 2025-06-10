#include "DiskConfig.h"
#include "tinyxml2.h"
#include <stdexcept>

DiskConfig loadDiskConfig(const std::string& path) {
    using namespace tinyxml2;
    DiskConfig config;
    XMLDocument doc;

    if (doc.LoadFile(path.c_str()) != XML_SUCCESS) {
        throw std::runtime_error("❌ No se pudo abrir el archivo XML: " + path);
    }

    auto* root = doc.FirstChildElement("disk");
    if (!root) throw std::runtime_error("❌ Falta <disk>");

    config.ip = root->FirstChildElement("ip")->GetText();
    root->FirstChildElement("port")->QueryIntText(&config.port);
    config.path = root->FirstChildElement("path")->GetText();

    unsigned int tempSize = 0, tempBlock = 0;
    root->FirstChildElement("size")->QueryUnsignedText(&tempSize);
    root->FirstChildElement("block_size")->QueryUnsignedText(&tempBlock);
    config.size = static_cast<size_t>(tempSize);
    config.blockSize = static_cast<size_t>(tempBlock);

    return config;
}
