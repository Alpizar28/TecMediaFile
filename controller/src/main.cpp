#include <crow.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <streambuf>
#include "FileController.h"
#include "Raid5.h"
#include "ControllerConfig.h"  // Incluye la definición de structs y la función

static std::string loadFileContent(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    return std::string(std::istreambuf_iterator<char>(in), {});
}

int main() {
    std::cout << "Iniciando TEC Media File System Controller...\n";

    ControllerConfig config = loadControllerConfig("controller/config/controller_config.xml");

    // Construir URLs de los discos
    std::vector<std::string> nodeUrls;
    for (const auto& d : config.disks)
        nodeUrls.push_back("http://" + d.ip + ":" + std::to_string(d.port));

    Raid5& raid = Raid5::getInstance();
    if (!raid.initialize(nodeUrls)) {
        std::cerr << "Error inicializando RAID 5\n";
        return 1;
    }

    crow::SimpleApp app;

    // Servir frontend
    CROW_ROUTE(app, "/")
    ([](const crow::request&, crow::response& res) {
        auto html = loadFileContent("frontend/index.html");
        if (html.empty()) {
            res.code = 404;
            res.write("index.html no encontrado");
        } else {
            res.set_header("Content-Type", "text/html");
            res.code = 200;
            res.write(html);
        }
        res.end();
    });

    CROW_ROUTE(app, "/static/<string>")
    ([](const crow::request&, crow::response& res, std::string file) {
        std::string path = "frontend/" + file;
        auto ext = std::filesystem::path(path).extension().string();
        auto content = loadFileContent(path);
        if (content.empty()) {
            res.code = 404;
            res.write("No encontrado: " + file);
        } else {
            if (ext == ".css") res.set_header("Content-Type", "text/css");
            else if (ext == ".js") res.set_header("Content-Type", "application/javascript");
            else res.set_header("Content-Type", "application/octet-stream");
            res.code = 200;
            res.write(content);
        }
        res.end();
    });

    FileController::registerRoutes(app);

    std::cout << "Servidor iniciado en puerto: " << config.controllerPort << "\n";
    std::cout << "Frontend en GET /\n";
    std::cout << "Recursos estáticos en /static/{file}\n";
    std::cout << "API: /upload, /list, /download, /delete, /status\n";

    app.port(config.controllerPort).multithreaded().run();
    return 0;
}
