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

    CROW_ROUTE(app, "/upload/<string>")
    .methods(crow::HTTPMethod::OPTIONS)
    ([&](const crow::request&, crow::response& res, std::string){
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "PUT,OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.code = 204;
        res.end();
    });

    // 2) GET /list - CORREGIDO PARA MOSTRAR TAMAÑOS REALES
    CROW_ROUTE(app, "/list")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res){
        crow::json::wvalue json;
        try {
            auto files = raid.listFiles();
            json = crow::json::wvalue::list();
            
            for (size_t i = 0; i < files.size(); ++i) {
                json[i] = crow::json::wvalue::object();
                json[i]["name"] = files[i];
                
                // CORRECCIÓN: Obtener el tamaño real del archivo
                try {
                    auto fileData = raid.retrieveFile(files[i]);
                    json[i]["size"] = static_cast<int64_t>(fileData.size());
                    std::cout << "Archivo: " << files[i] << " - Tamaño: " << fileData.size() << " bytes\n";
                } catch (...) {
                    json[i]["size"] = 0;
                    std::cout << "No se pudo obtener tamaño de: " << files[i] << "\n";
                }
            }
            
            res.code = 200;
            std::cout << "Lista de archivos enviada: " << files.size() << " archivos\n";
        } catch (const std::exception& e) {
            json = crow::json::wvalue::list();
            res.code = 500;
            std::cerr << "Error en /list: " << e.what() << "\n";
        } catch (...) {
            json = crow::json::wvalue::list();
            res.code = 500;
            std::cerr << "Error desconocido en /list\n";
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

    // 3) GET /download/<filename>
    CROW_ROUTE(app, "/download/<string>")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res, std::string filename){
        try {
            std::cout << "Intentando descargar archivo: " << filename << "\n";
            auto data = raid.retrieveFile(filename);
            if (data.empty()) {
                res.code = 404;
                res.write("Archivo no encontrado: " + filename);
                std::cout << "Archivo no encontrado: " << filename << "\n";
            } else {
                std::string body(reinterpret_cast<const char*>(data.data()), data.size());
                res.code = 200;
                res.set_header("Content-Type", "application/octet-stream");
                res.set_header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
                res.set_header("Content-Length", std::to_string(data.size()));
                res.write(body);
                std::cout << "Archivo descargado exitosamente: " << filename << " (" << data.size() << " bytes)\n";
            }
        } catch (const std::exception& e) {
            res.code = 500;
            res.write("Error de descarga: " + std::string(e.what()));
            std::cerr << "Error en descarga de " << filename << ": " << e.what() << "\n";
        } catch (...) {
            res.code = 500;
            res.write("Error desconocido de descarga");
            std::cerr << "Error desconocido en descarga de " << filename << "\n";
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

    // 4) DELETE /delete/<filename>
    CROW_ROUTE(app, "/delete/<string>")
    .methods(crow::HTTPMethod::DELETE)
    ([&](const crow::request&, crow::response& res, std::string filename){
        try {
            std::cout << "Intentando eliminar archivo: " << filename << "\n";
            bool success = raid.deleteFile(filename);
            if (success) {
                res.code = 200;
                res.write("Archivo eliminado: " + filename);
                std::cout << "Archivo eliminado exitosamente: " << filename << "\n";
                
                // CORRECCIÓN: Limpiar nodos después de eliminar archivo
                try {
                    raid.cleanupDeletedFile(filename);
                    std::cout << "Limpieza de nodos completada para: " << filename << "\n";
                } catch (...) {
                    std::cout << "Advertencia: No se pudo limpiar nodos para: " << filename << "\n";
                }
            } else {
                res.code = 404;
                res.write("Archivo no encontrado: " + filename);
                std::cout << "Archivo no encontrado para eliminar: " << filename << "\n";
            }
        } catch (const std::exception& e) {
            res.code = 500;
            res.write("Error al eliminar archivo: " + std::string(e.what()));
            std::cerr << "Error eliminando " << filename << ": " << e.what() << "\n";
        } catch (...) {
            res.code = 500;
            res.write("Error desconocido al eliminar archivo");
            std::cerr << "Error desconocido eliminando " << filename << "\n";
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

    // 5) GET /status - MEJORADO
    CROW_ROUTE(app, "/status")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res){
        crow::json::wvalue json;
        try {
            std::cout << "Obteniendo estado de nodos...\n";
            
            auto files = raid.listFiles();
            std::cout << "Archivos encontrados: " << files.size() << "\n";
            
            auto statusMap = raid.getStatusPerFile();
            std::cout << "StatusMap tiene " << statusMap.size() << " entradas\n";
            
            // CORRECCIÓN: Solo incluir archivos que realmente existen
            for (const auto& filename : files) {
                if (statusMap.find(filename) != statusMap.end()) {
                    const auto& nodes = statusMap[filename];
                    json[filename] = crow::json::wvalue::list();
                    
                    std::cout << "Procesando archivo: " << filename << " con " << nodes.size() << " nodos\n";
                    for (size_t i = 0; i < nodes.size(); ++i) {
                        json[filename][i] = nodes[i].isActive;
                        std::cout << "  Nodo " << nodes[i].nodeId << ": " << (nodes[i].isActive ? "activo" : "inactivo") << "\n";
                    }
                } else {
                    std::cout << "ADVERTENCIA: Archivo " << filename << " no tiene estado de nodos\n";
                    // Crear estado por defecto
                    json[filename] = crow::json::wvalue::list();
                    for (int i = 0; i < 4; ++i) {
                        json[filename][i] = true; // Todos activos por defecto
                    }
                }
            }
            
            res.code = 200;
            std::cout << "Estado enviado exitosamente\n";
            
        } catch (const std::exception& e) {
            json["error"] = std::string("Error al obtener estado: ") + e.what();
            res.code = 500;
            std::cerr << "Error en /status: " << e.what() << "\n";
        } catch (...) {
            json["error"] = "Error desconocido al obtener estado";
            res.code = 500;
            std::cerr << "Error desconocido en /status\n";
        }
        
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Content-Type", "application/json");
        std::string jsonStr = json.dump();
        std::cout << "JSON de respuesta /status: " << jsonStr.substr(0, 200) << "...\n";
        res.write(jsonStr);
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

    // 6) POST /recover/<nodeId>
    CROW_ROUTE(app, "/recover/<int>")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request&, crow::response& res, int nodeId){
        crow::json::wvalue json;
        try {
            std::cout << "Intentando recuperar nodo: " << nodeId << "\n";
            bool ok = raid.recoverFromFailure(nodeId);
            json["node_id"] = nodeId;
            json["status"] = ok ? "success" : "failed";
            json["message"] = ok ? "Nodo recuperado exitosamente" : "Falló la recuperación del nodo";
            
            res.code = ok ? 200 : 500;
            std::cout << "Recuperación de nodo " << nodeId << ": " << (ok ? "exitosa" : "fallida") << "\n";
            
        } catch (const std::exception& e) {
            json["node_id"] = nodeId;
            json["status"] = "error";
            json["message"] = std::string("Error en recuperación: ") + e.what();
            res.code = 500;
            std::cerr << "Error recuperando nodo " << nodeId << ": " << e.what() << "\n";
        } catch (...) {
            json["node_id"] = nodeId;
            json["status"] = "error";
            json["message"] = "Error desconocido en recuperación";
            res.code = 500;
            std::cerr << "Error desconocido recuperando nodo " << nodeId << "\n";
        }
        
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

    // 7) ENDPOINT DE DEBUG MEJORADO
    CROW_ROUTE(app, "/debug")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request&, crow::response& res){
        crow::json::wvalue json;
        try {
            json["timestamp"] = std::time(nullptr);
            json["server_status"] = "running";
            
            auto files = raid.listFiles();
            json["total_files"] = files.size();
            json["files"] = crow::json::wvalue::list();
            
            for (size_t i = 0; i < files.size(); ++i) {
                json["files"][i] = crow::json::wvalue::object();
                json["files"][i]["name"] = files[i];
                
                try {
                    auto fileData = raid.retrieveFile(files[i]);
                    json["files"][i]["size"] = static_cast<int64_t>(fileData.size());
                } catch (...) {
                    json["files"][i]["size"] = -1; // Error
                }
            }
            
            res.code = 200;
        } catch (...) {
            json["error"] = "Error en debug";
            res.code = 500;
        }
        
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Content-Type", "application/json");
        res.write(json.dump());
        res.end();
    });

    // 8) NUEVO ENDPOINT PARA LIMPIAR NODOS HUÉRFANOS
    CROW_ROUTE(app, "/cleanup")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request&, crow::response& res){
        crow::json::wvalue json;
        try {
            std::cout << "Iniciando limpieza de nodos huérfanos...\n";
            
            // Limpiar archivos que ya no existen
            int cleaned = raid.cleanupOrphanedNodes();
            
            json["status"] = "success";
            json["cleaned_nodes"] = cleaned;
            json["message"] = "Limpieza completada";
            
            res.code = 200;
            std::cout << "Limpieza completada: " << cleaned << " nodos eliminados\n";
            
        } catch (const std::exception& e) {
            json["status"] = "error";
            json["message"] = std::string("Error en limpieza: ") + e.what();
            res.code = 500;
            std::cerr << "Error en limpieza: " << e.what() << "\n";
        } catch (...) {
            json["status"] = "error";
            json["message"] = "Error desconocido en limpieza";
            res.code = 500;
            std::cerr << "Error desconocido en limpieza\n";
        }
        
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Content-Type", "application/json");
        res.write(json.dump());
        res.end();
    });
}