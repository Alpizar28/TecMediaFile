#include <crow.h>
#include <filesystem>
#include <fstream>
#include <streambuf>
#include <iostream>
#include "FileController.h"
#include "Raid5.h"

static std::string loadFileContent(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    return std::string(std::istreambuf_iterator<char>(in), {});
}

int main() {
    std::cout << "Iniciando TEC Media File System Controller...\n";

    // Inicializar RAID 5
    std::vector<std::string> nodeUrls = {
        "http://localhost:18081",
        "http://localhost:18082",
        "http://localhost:18083",
        "http://localhost:18084"
    };
    Raid5& raid = Raid5::getInstance();
    if (!raid.initialize(nodeUrls)) {
        std::cerr << "Error inicializando RAID 5\n";
        return 1;
    }

    crow::SimpleApp app;

    //
    // Servir index.html en GET /
    //
    CROW_ROUTE(app, "/")
    .methods(crow::HTTPMethod::GET)
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

    //
    // Servir CSS y JS en GET /static/{filename}
    //
    CROW_ROUTE(app, "/static/<string>")
    .methods(crow::HTTPMethod::GET)
    ([](const crow::request&, crow::response& res, std::string file) {
        std::string path = "frontend/" + file;
        auto ext = std::filesystem::path(path).extension().string();
        auto content = loadFileContent(path);
        if (content.empty()) {
            res.code = 404;
            res.write("No encontrado: " + file);
        } else {
            if (ext == ".css") {
                res.set_header("Content-Type", "text/css");
            } else if (ext == ".js") {
                res.set_header("Content-Type", "application/javascript");
            } else {
                res.set_header("Content-Type", "application/octet-stream");
            }
            res.code = 200;
            res.write(content);
        }
        res.end();
    });

    // Registrar rutas REST
    FileController::registerRoutes(app);

    std::cout << "Servidor iniciado en puerto 18080\n";
    std::cout << "Frontend en GET /\n";
    std::cout << "Recursos estáticos en GET /static/{filename}\n";
    std::cout << "Rutas API: /upload, /list, /download, /delete, /status\n";

    app.port(18080).multithreaded().run();
    return 0;
}
