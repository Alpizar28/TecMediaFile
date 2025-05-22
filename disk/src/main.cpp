#include <crow.h>
#include <nlohmann/json.hpp>
#include "DiskNode.h"

int main() {
    crow::SimpleApp app;

    // POST /write/<id>
    CROW_ROUTE(app, "/write/<int>")
      .methods(crow::HTTPMethod::Post)
    ([](crow::request& req, crow::response& res, int id){
        auto j = nlohmann::json::parse(req.body);
        bool ok = DiskNode::instance().writeBlock(id, j["data"]);
        res.code = ok ? 200 : 500;
        res.end();
    });

    // GET /read/<id>
    CROW_ROUTE(app, "/read/<int>")
    ([](int id){
        std::string data = DiskNode::instance().readBlock(id);
        nlohmann::json resp{ {"data", data} };
        return crow::response{ resp.dump() };
    });

    app.port(18081)
       .multithreaded()
       .run();

    return 0;
}
