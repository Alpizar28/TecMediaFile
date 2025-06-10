#include "../include/ControllerConfig.h"
#include "tinyxml2.h"
#include <stdexcept>
#include <iostream>

ControllerConfig loadControllerConfig(const std::string& path) {
    using namespace tinyxml2;
    ControllerConfig config;
    XMLDocument doc;

    if (doc.LoadFile(path.c_str()) != XML_SUCCESS) {
        throw std::runtime_error("❌ No se pudo abrir el archivo XML: " + path);
    }

    auto* root = doc.FirstChildElement("controller");
    if (!root) throw std::runtime_error("❌ Falta <controller>");

    auto* portElem = root->FirstChildElement("port");
    if (!portElem) throw std::runtime_error("❌ Falta <port>");
    portElem->QueryIntText(&config.controllerPort);

    auto* disks = root->FirstChildElement("disks");
    if (!disks) throw std::runtime_error("❌ Falta <disks>");

    for (auto* disk = disks->FirstChildElement("disk"); disk; disk = disk->NextSiblingElement("disk")) {
        auto* ipElem = disk->FirstChildElement("ip");
        auto* portElemDisk = disk->FirstChildElement("port");
        if (!ipElem || !portElemDisk) throw std::runtime_error("❌ Falta <ip> o <port> en <disk>");

        DiskInfo d;
        d.ip = ipElem->GetText();
        portElemDisk->QueryIntText(&d.port);
        config.disks.push_back(d);
    }

    return config;
}
