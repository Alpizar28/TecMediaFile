#include <crow.h>
#include <filesystem>
#include "FileController.h"

int main()
{
    crow::SimpleApp app;

    // Registrar rutas
    FileController::registerRoutes(app);

    // Levantar el servidor
    app.port(18080)
       .multithreaded()
       .run();
    return 0;
}
