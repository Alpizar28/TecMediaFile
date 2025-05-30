#include "FileController.h"
#include "Raid5.h"
#include <vector>
#include <iostream>

void FileController::registerRoutes(crow::SimpleApp& app) {
    Raid5& raid = Raid5::getInstance();

    //
    // 1) PUT /upload/<filename> + OPTIONS
    //
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
                std::cout << "Archivo " << filename << " almacenado exitosamente\n";
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

    CROW_ROUTE(app, "/upload/<string>")
    .methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res, std::string){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "PUT,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204;
        res.end();
    });

    //
    // 2) GET /list + OPTIONS
    //
    CROW_ROUTE(app, "/list")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res){
        crow::json::wvalue json;
        try {
            auto files = raid.listFiles();
            json["count"] = files.size();
            json["files"] = crow::json::wvalue::list();
            for (size_t i = 0; i < files.size(); ++i)
                json["files"][i] = files[i];
            res.code = 200;
        } catch (...) {
            json["error"] = "Error listando archivos";
            json["count"] = 0;
            json["files"] = crow::json::wvalue::list();
            res.code = 500;
        }
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Content-Type", "application/json");
        res.write(json.dump());
        res.end();
    });

    CROW_ROUTE(app, "/list")
    .methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204;
        res.end();
    });

    //
    // 3) GET /download/<filename> + OPTIONS
    //
    CROW_ROUTE(app, "/download/<string>")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res, std::string filename){
        try {
            auto data = raid.retrieveFile(filename);
            if (data.empty()) {
                res.code = 404;
                res.write("Archivo no encontrado: " + filename);
                std::cerr << "Bloque no encontrado para " << filename << "\n";
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
            res.write("Error de descarga");
        }
        res.add_header("Access-Control-Allow-Origin", "*");
        res.end();
    });

    CROW_ROUTE(app, "/download/<string>")
    .methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res, std::string){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204;
        res.end();
    });

    //
    // 4) DELETE /delete/<filename> + OPTIONS
    //
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

    CROW_ROUTE(app, "/delete/<string>")
    .methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res, std::string){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "DELETE,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204;
        res.end();
    });

    //
    // 5) GET /status + OPTIONS
    //
    CROW_ROUTE(app, "/status")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res){
        crow::json::wvalue json;
        try {
            auto st = raid.getStatus();
            json["nodes"] = crow::json::wvalue::list();
            for (size_t i = 0; i < st.size(); ++i) {
                json["nodes"][i]["id"]          = st[i].nodeId;
                json["nodes"][i]["status"]      = st[i].isActive ? "active" : "inactive";
                json["nodes"][i]["blocks_used"] = st[i].blocksUsed;
                json["nodes"][i]["total_blocks"] = st[i].totalBlocks;
            }
            res.code = 200;
        } catch (...) {
            json["error"] = "Error al obtener estado";
            res.code = 500;
        }
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Content-Type", "application/json");
        res.write(json.dump());
        res.end();
    });

    CROW_ROUTE(app, "/status")
    .methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204;
        res.end();
    });

    //
    // 6) POST /recover/<nodeId> + OPTIONS
    //
    CROW_ROUTE(app, "/recover/<int>")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request&, crow::response& res, int nodeId){
        bool ok = raid.recoverFromFailure(nodeId);
        crow::json::wvalue json;
        json["node_id"] = nodeId;
        json["status"]  = ok ? "success" : "failed";
        res.code = ok ? 200 : 500;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Content-Type", "application/json");
        res.write(json.dump());
        res.end();
    });

    CROW_ROUTE(app, "/recover/<int>")
    .methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res, int){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "POST,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204;
        res.end();
    });
}
