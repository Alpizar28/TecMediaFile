#include <crow.h>
#include <nlohmann/json.hpp>
#include "Controller.h"

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/add/<string>")
    .methods(crow::HTTPMethod::Post)
    ([](const std::string& name){
        bool ok = Controller::instance().addDocument(name, "");
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
