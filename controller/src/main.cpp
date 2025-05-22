#include <crow.h>
#include <nlohmann/json.hpp>
#include "Controller.h"

int main() {
    crow::SimpleApp app;

    // Añadir documento: recibe nombre y datos en base64
    CROW_ROUTE(app, "/add")
      .methods(crow::HTTPMethod::Post)
    ([](const crow::request& req){
        auto j = nlohmann::json::parse(req.body);
        std::string name = j["name"];
        std::string data_b64 = j["data"];
        bool ok = Controller::instance().addDocument(name, data_b64);
        nlohmann::json resp{ {"ok", ok} };
        return crow::response{ resp.dump() };
    });

    // Listar documentos existentes
    CROW_ROUTE(app, "/list")
      ([]{
        auto docs = Controller::instance().listDocuments();
        nlohmann::json resp = docs;
        return crow::response{ resp.dump() };
    });

    // Descargar documento completo (ensamblado)
    CROW_ROUTE(app, "/download/<string>")
      ([](const std::string& name){
        auto assembled = Controller::instance().downloadDocument(name);
        nlohmann::json resp{ {"data", assembled} };
        return crow::response{ resp.dump() };
    });

    app.port(18080)
       .multithreaded()
       .run();
    return 0;
}
