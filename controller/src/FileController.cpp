#include "FileController.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

void FileController::registerRoutes(crow::SimpleApp& app) {

    // PUT /upload/<filename>
    CROW_ROUTE(app, "/upload/<string>")
        .methods(crow::HTTPMethod::PUT)
    ([](const crow::request& req, crow::response& res, std::string filename){
        std::filesystem::create_directories("storage");
        std::ofstream ofs("storage/" + filename, std::ios::binary);
        ofs.write(req.body.data(), req.body.size());
        ofs.close();

        res.code = 200;
        res.write("Archivo subido como " + filename);
        res.add_header("Access-Control-Allow-Origin", "*");
        res.end();
    });

    // GET /list
    CROW_ROUTE(app, "/list")
    ([&](){
        std::filesystem::create_directories("storage");
        crow::json::wvalue json;
        json["files"] = crow::json::wvalue::list();
        int idx = 0;
        for (auto& p : fs::directory_iterator("storage")) {
            if (p.path().extension() == ".pdf") {
                json["files"][idx++] = p.path().filename().string();
            }
        }
        crow::response res{ json };
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // GET /download/<string>
    CROW_ROUTE(app, "/download/<string>")
    ([](const crow::request&, crow::response& res, std::string name){
        auto path = fs::path("storage") / name;
        if (!fs::exists(path)) {
            res.code = 404;
            res.write("No existe: " + name);
        } else {
            std::ifstream ifs(path, std::ios::binary);
            std::ostringstream ss;
            ss << ifs.rdbuf();
            res.add_header("Content-Type", "application/octet-stream");
            res.write(ss.str());
        }
        res.add_header("Access-Control-Allow-Origin", "*");
        res.end();
    });

    // DELETE /delete/<string>
    CROW_ROUTE(app, "/delete/<string>")
        .methods(crow::HTTPMethod::DELETE)
    ([](const crow::request&, crow::response& res, std::string name){
        auto path = fs::path("storage") / name;
        if (fs::remove(path)) {
            res.code = 200;
            res.write("Eliminado: " + name);
        } else {
            res.code = 404;
            res.write("No encontrado: " + name);
        }
        res.add_header("Access-Control-Allow-Origin", "*");
        res.end();
    });

    // OPTIONS for CORS preflight - CORREGIDO: ahora acepta el parámetro path
    CROW_ROUTE(app, "/<path>")
        .methods(crow::HTTPMethod::OPTIONS)
    ([](const crow::request&, crow::response& res, std::string path){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET,PUT,DELETE,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204;
        res.end();
    });
}