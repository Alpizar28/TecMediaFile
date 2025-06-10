#include <crow.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include "DiskNode.h"
#include "DiskConfig.h"  // <--- acá está el cambio clave

int main() {
    // Leemos la configuración desde el XML
    DiskConfig config = loadDiskConfig("disk/config/disk_config.xml");

    std::cout << "🟢 DiskNode iniciado en IP: " << config.ip
              << ", Puerto: " << config.port << "\n";
    std::cout << "📁 Usando carpeta: " << config.path
              << ", Tamaño total: " << config.size
              << ", Tamaño de bloque: " << config.blockSize << "\n";

    crow::SimpleApp app;

    // Ruta POST /write/<id>
    CROW_ROUTE(app, "/write/<int>").methods(crow::HTTPMethod::Post)
    ([](crow::request& req, crow::response& res, int id) {
        auto j = nlohmann::json::parse(req.body);
        bool ok = DiskNode::instance().writeBlock(id, j["data"]);
        res.code = ok ? 200 : 500;
        res.end();
    });

    // Ruta GET /read/<id>
    CROW_ROUTE(app, "/read/<int>")
    ([](int id) {
        std::string data = DiskNode::instance().readBlock(id);
        nlohmann::json resp{{"data", data}};
        return crow::response{resp.dump()};
    });

    // Iniciar el servidor en el puerto leído del XML
    app.port(config.port).multithreaded().run();
    return 0;
}
