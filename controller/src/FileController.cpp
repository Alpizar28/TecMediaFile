#include "FileController.h"
#include "Raid5.h"
#include <vector>
#include <iostream>

void FileController::registerRoutes(crow::SimpleApp& app) {
    Raid5& raid = Raid5::getInstance();

    // 1) PUT /upload/<filename>
    CROW_ROUTE(app, "/upload/<string>")
    .methods(crow::HTTPMethod::PUT)
    ([&](const crow::request& req, crow::response& res, std::string filename){
        if (req.body.empty()) {
            res.code = 400;
            res.write("Error: Archivo vacío");
        } else {
            std::vector<uint8_t> data(req.body.begin(), req.body.end());
            bool success = raid.storeFile(filename, data);
            if (success) {
                res.code = 200;
                res.write("Upload OK: " + filename + " (" + std::to_string(data.size()) + " bytes)");
                std::cout << "Archivo " << filename << " almacenado exitosamente (" << data.size() << " bytes)\n";
            } else {
                res.code = 500;
                res.write("Upload ERROR: No se pudo almacenar " + filename);
                std::cerr << "Error al almacenar archivo " << filename << "\n";
            }
        }
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.end();
    });

    // 2) GET /list
    CROW_ROUTE(app, "/list")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res){
        crow::json::wvalue json = crow::json::wvalue::list();
        try {
            auto files = raid.listFiles();
            for (size_t i = 0; i < files.size(); ++i) {
                json[i] = crow::json::wvalue::object();
                json[i]["name"] = files[i];
                try {
                    auto data = raid.retrieveFile(files[i]);
                    json[i]["size"] = static_cast<int64_t>(data.size());
                    std::cout << "Archivo: " << files[i] << " - Tamaño: " << data.size() << " bytes\n";
                } catch (...) {
                    json[i]["size"] = 0;
                }
            }
            res.code = 200;
        } catch (...) {
            res.code = 500;
        }
        res.add_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");
        res.write(json.dump());
        res.end();
    });

    CROW_ROUTE(app, "/list").methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204; res.end();
    });

    // 3) GET /download/<filename>
    CROW_ROUTE(app, "/download/<string>")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res, std::string filename){
        try {
            auto data = raid.retrieveFile(filename);
            if (data.empty()) {
                res.code = 404;
                res.write("Archivo no encontrado: " + filename);
            } else {
                std::string body(reinterpret_cast<const char*>(data.data()), data.size());
                res.code = 200;
                res.set_header("Content-Type", "application/octet-stream");
                res.set_header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
                res.set_header("Content-Length", std::to_string(data.size()));
                res.write(body);
            }
        } catch (...) {
            res.code = 500;
            res.write("Error al descargar: " + filename);
        }
        res.add_header("Access-Control-Allow-Origin", "*");
        res.end();
    });

    CROW_ROUTE(app, "/download/<string>").methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res, std::string){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204; res.end();
    });

    // 4) DELETE /delete/<filename>
    CROW_ROUTE(app, "/delete/<string>")
    .methods(crow::HTTPMethod::DELETE)
    ([&](const crow::request&, crow::response& res, std::string filename){
        bool success = raid.deleteFile(filename);
        if (success) {
            res.code = 200;
            res.write("Archivo eliminado: " + filename);
        } else {
            res.code = 404;
            res.write("Archivo no encontrado: " + filename);
        }
        res.add_header("Access-Control-Allow-Origin", "*");
        res.end();
    });

    CROW_ROUTE(app, "/delete/<string>").methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res, std::string){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "DELETE,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204; res.end();
    });

    // 5) GET /status
    CROW_ROUTE(app, "/status").methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res){
        crow::json::wvalue json;
        auto statusMap = raid.getStatusPerFile();
        for (const auto& [file, nodes] : statusMap) {
            json[file] = crow::json::wvalue::list();
            for (size_t i = 0; i < nodes.size(); ++i)
                json[file][i] = nodes[i].isActive;
        }
        res.code = 200;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");
        res.write(json.dump());
        res.end();
    });

    CROW_ROUTE(app, "/status").methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204; res.end();
    });


    // 6) GET /nodes/<filename> - listar nodos activos
    CROW_ROUTE(app, "/nodes/<string>")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res, std::string filename){
        auto statusMap = raid.getStatusPerFile();
        auto status = statusMap[filename];
        crow::json::wvalue json;
        json["filename"] = filename;
        json["active_nodes"] = crow::json::wvalue::list();
        int idx = 0;
        for (const auto& node : status) {
            if (node.isActive) {
                json["active_nodes"][idx++] = node.nodeId;
            }
        }
        res.code = 200;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");
        res.write(json.dump());
        res.end();
    });

    // 7 DELETE /delete_node/<filename>/<nodeId> - borrar nodo específico
    CROW_ROUTE(app, "/delete_node/<string>/<int>")
    .methods(crow::HTTPMethod::DELETE)
    ([&](const crow::request&, crow::response& res, std::string filename, int nodeId){
        bool ok = raid.disableNode(nodeId);
        crow::json::wvalue json;
        if (ok) {
            json["message"] = "Nodo " + std::to_string(nodeId)
                               + " de '" + filename + "' eliminado exitosamente";
            res.code = 200;
        } else {
            json["error"] = "No se pudo eliminar nodo " + std::to_string(nodeId)
                             + " de '" + filename + "'";
            res.code = 400;
        }
        res.add_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");
        res.write(json.dump());
        res.end();
    });

    CROW_ROUTE(app, "/delete_node/<string>/<int>").methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res, std::string, int){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "DELETE,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204; res.end();
    });

    // PUT /enable_node/<string>/<int> — restaurar un nodo para un archivo
    CROW_ROUTE(app, "/enable_node/<string>/<int>")
    .methods(crow::HTTPMethod::PUT)
    ([&](const std::string& filename, int nodeId){
        bool ok = Raid5::getInstance().enableNode(nodeId);
        crow::json::wvalue json;
        if (ok) {
        json["message"] = "Nodo " + std::to_string(nodeId)
                        + " de '" + filename + "' restaurado exitosamente";
        std::cout << "Nodo " << nodeId << " de '" << filename
                    << "' restaurado exitosamente\n";
        return crow::response{json};
        } else {
        json["error"] = "No se pudo restaurar el nodo "
                        + std::to_string(nodeId)
                        + " de '" + filename + "'";
        return crow::response{400, json};
        }
    });

}
