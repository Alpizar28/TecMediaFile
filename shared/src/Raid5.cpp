// shared/src/Raid5.cpp
#include "Raid5.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <ctime>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

// Instancia singleton
Raid5& Raid5::getInstance() {
    static Raid5 instance;
    return instance;
}

// Inicializa RAID 5 y crea carpetas node0…nodeN
bool Raid5::initialize(const std::vector<std::string>& nodeUrls) {
    if (nodeUrls.size() != RAID_NODES) {
        std::cerr << "RAID 5 requiere exactamente " << RAID_NODES << " nodos\n";
        return false;
    }
    nodeUrls_ = nodeUrls;
    
    // Inicializar estado de nodos (todos activos por defecto)
    disabledNodes_.clear();

    // Crear directorios para simular nodos
    for (int i = 0; i < RAID_NODES; ++i) {
        std::filesystem::create_directories("node" + std::to_string(i));
    }

    // Verificar salud básica
    for (int i = 0; i < RAID_NODES; ++i) {
        if (!isNodeActive(i)) {
            std::cerr << "Nodo " << i << " no está activo\n";
            return false;
        }
    }
    std::cout << "RAID 5 inicializado con " << RAID_NODES << " nodos\n";
    return true;
}

// NUEVA FUNCIÓN: Deshabilitar un nodo
bool Raid5::disableNode(int nodeId) {
    if (nodeId < 0 || nodeId >= RAID_NODES) {
        std::cerr << "Error: ID de nodo inválido " << nodeId << "\n";
        return false;
    }
    
    // Verificar si el nodo ya está deshabilitado
    if (disabledNodes_.find(nodeId) != disabledNodes_.end()) {
        std::cout << "Nodo " << nodeId << " ya está deshabilitado\n";
        return true;
    }
    
    // Verificar si deshabilitar este nodo mantendría el sistema funcional
    int activeNodes = RAID_NODES - disabledNodes_.size();
    if (activeNodes <= 3) { // RAID 5 necesita al menos 3 nodos
        std::cerr << "Error: No se puede deshabilitar el nodo " << nodeId 
                  << ". El sistema necesita al menos 3 nodos activos\n";
        return false;
    }
    
    // Agregar el nodo a la lista de deshabilitados
    disabledNodes_.insert(nodeId);
    
    std::cout << "Nodo " << nodeId << " deshabilitado exitosamente\n";
    std::cout << "Nodos activos restantes: " << (RAID_NODES - disabledNodes_.size()) << "\n";
    
    return true;
}

// NUEVA FUNCIÓN: Habilitar un nodo previamente deshabilitado
bool Raid5::enableNode(int nodeId) {
    if (nodeId < 0 || nodeId >= RAID_NODES) {
        std::cerr << "Error: ID de nodo inválido " << nodeId << "\n";
        return false;
    }
    
    auto it = disabledNodes_.find(nodeId);
    if (it == disabledNodes_.end()) {
        std::cout << "Nodo " << nodeId << " ya está habilitado\n";
        return true;
    }
    
    // Eliminar el nodo de la lista de deshabilitados
    disabledNodes_.erase(it);
    
    // Crear directorio del nodo si no existe
    std::filesystem::create_directories("node" + std::to_string(nodeId));
    
    std::cout << "Nodo " << nodeId << " habilitado exitosamente\n";
    return true;
}

// NUEVA FUNCIÓN: Obtener lista de nodos deshabilitados
std::vector<int> Raid5::getDisabledNodes() const {
    return std::vector<int>(disabledNodes_.begin(), disabledNodes_.end());
}

// Almacenar archivo en bloques y paridad
bool Raid5::storeFile(const std::string& filename, const std::vector<uint8_t>& data) {
    if (data.empty()) {
        std::cerr << "Error: datos vacíos para archivo " << filename << "\n";
        return false;
    }
    
    // Verificar que hay suficientes nodos activos
    if (!isSystemHealthy()) {
        std::cerr << "Error: Sistema no saludable, no se puede almacenar archivo\n";
        return false;
    }
    
    if (fileExists(filename)) {
        deleteFile(filename);
    }

    auto blocks = splitIntoBlocks(data);
    FileMetadata meta;
    meta.filename   = filename;
    meta.fileSize   = data.size();
    meta.createdAt  = std::time(nullptr);
    meta.modifiedAt = meta.createdAt;

    for (size_t i = 0; i < blocks.size(); ++i) {
        int group      = i / (RAID_NODES - 1);
        int offset     = i % (RAID_NODES - 1);
        int parityNode = getParityNode(group);
        int dataNode   = (offset >= parityNode ? offset + 1 : offset);
        int blockId    = nextBlockId_++;

        // Verificar si el nodo de datos está activo
        if (!isNodeActive(dataNode)) {
            std::cerr << "Error: Nodo de datos " << dataNode << " no está activo\n";
            return false;
        }

        if (!writeBlockToNode(dataNode, blockId, blocks[i])) {
            std::cerr << "Error escribiendo bloque " << blockId << "\n";
            return false;
        }
        meta.blockIds.push_back(blockId);

        // Calcular y escribir bloque de paridad al final de cada grupo
        if ((i + 1) % (RAID_NODES - 1) == 0 || i + 1 == blocks.size()) {
            // Verificar si el nodo de paridad está activo
            if (!isNodeActive(parityNode)) {
                std::cerr << "Error: Nodo de paridad " << parityNode << " no está activo\n";
                return false;
            }
            
            std::vector<std::vector<uint8_t>> grp;
            int start = group * (RAID_NODES - 1);
            for (int j = start; j <= static_cast<int>(i); ++j)
                grp.push_back(blocks[j]);
            auto parity = calculateParity(grp);
            int pid = nextBlockId_++;
            if (!writeBlockToNode(parityNode, pid, parity)) {
                std::cerr << "Error escribiendo paridad " << pid << "\n";
                return false;
            }
            meta.parityBlockId = pid;
        }
    }

    fileMetadata_[filename] = std::move(meta);
    return true;
}

// Recuperar archivo
std::vector<uint8_t> Raid5::retrieveFile(const std::string& filename) {
    auto it = fileMetadata_.find(filename);
    if (it == fileMetadata_.end()) return {};

    const auto& meta = it->second;
    std::vector<uint8_t> out;
    out.reserve(meta.fileSize);

    for (int bid : meta.blockIds) {
        bool ok = false;
        for (int node = 0; node < RAID_NODES && !ok; ++node) {
            if (!isNodeActive(node)) continue; // Saltar nodos deshabilitados
            
            auto blk = readBlockFromNode(node, bid);
            if (!blk.empty()) {
                out.insert(out.end(), blk.begin(), blk.end());
                ok = true;
            }
        }
        if (!ok) {
            auto rec = recoverBlock(bid, -1);
            if (rec.empty()) return {};
            out.insert(out.end(), rec.begin(), rec.end());
        }
    }

    if (out.size() > meta.fileSize) out.resize(meta.fileSize);
    return out;
}

// Método alias para compatibilidad con FileController
std::vector<uint8_t> Raid5::loadFile(const std::string& filename) {
    return retrieveFile(filename);
}

bool Raid5::deleteFile(const std::string& filename) {
    auto it = fileMetadata_.find(filename);
    if (it == fileMetadata_.end()) return false;
    
    // Limpiar los bloques físicos antes de eliminar los metadatos
    cleanupDeletedFile(filename);
    
    fileMetadata_.erase(it);
    return true;
}

std::vector<std::string> Raid5::listFiles() const {
    std::vector<std::string> names;
    names.reserve(fileMetadata_.size());
    for (const auto& p : fileMetadata_) names.push_back(p.first);
    return names;
}

std::vector<NodeStatus> Raid5::getStatus() const {
    std::vector<NodeStatus> st;
    st.reserve(RAID_NODES);
    for (int i = 0; i < RAID_NODES; ++i) {
        NodeStatus ns{i, isNodeActive(i), 0, 1000};
        for (const auto& fm : fileMetadata_)
            for (int bid : fm.second.blockIds)
                if (bid % RAID_NODES == i) ns.blocksUsed++;
        st.push_back(ns);
    }
    return st;
}

bool Raid5::isSystemHealthy() const {
    auto status = getStatus();
    int activeNodes = 0;
    
    for (const auto& nodeStatus : status) {
        if (nodeStatus.isActive) {
            activeNodes++;
        }
    }
    
    // RAID 5 requiere al menos 3 nodos activos para operar
    return activeNodes >= 3;
}

bool Raid5::recoverFromFailure(int failedNodeId) {
    if (failedNodeId < 0 || failedNodeId >= RAID_NODES) {
        return false;
    }
    
    // Implementación básica de recuperación
    std::cout << "Recuperando nodo " << failedNodeId << "\n";
    
    // Habilitar el nodo si estaba deshabilitado
    enableNode(failedNodeId);
    
    // Crear directorio del nodo si no existe
    std::filesystem::create_directories("node" + std::to_string(failedNodeId));
    
    // Aquí se implementaría la lógica de recuperación real
    // Por ahora, solo retornamos true si el sistema sigue siendo saludable
    return isSystemHealthy();
}

size_t Raid5::getFileSize(const std::string& fn) const {
    auto it = fileMetadata_.find(fn);
    return it == fileMetadata_.end() ? 0 : it->second.fileSize;
}

bool Raid5::fileExists(const std::string& fn) const {
    return fileMetadata_.count(fn) > 0;
}

// NUEVOS MÉTODOS DE LIMPIEZA
bool Raid5::cleanupDeletedFile(const std::string& filename) {
    auto it = fileMetadata_.find(filename);
    if (it == fileMetadata_.end()) {
        return true; // El archivo ya no existe en metadatos
    }

    const auto& meta = it->second;
    bool success = true;

    // Eliminar todos los bloques de datos
    for (int blockId : meta.blockIds) {
        for (int nodeId = 0; nodeId < RAID_NODES; ++nodeId) {
            if (!removeBlockFromNode(nodeId, blockId)) {
                success = false;
            }
        }
    }

    // Eliminar bloque de paridad
    if (meta.parityBlockId >= 0) {
        for (int nodeId = 0; nodeId < RAID_NODES; ++nodeId) {
            if (!removeBlockFromNode(nodeId, meta.parityBlockId)) {
                success = false;
            }
        }
    }

    return success;
}

int Raid5::cleanupOrphanedNodes() {
    auto orphanedBlocks = findOrphanedBlocks();
    int cleanedCount = 0;

    for (int blockId : orphanedBlocks) {
        for (int nodeId = 0; nodeId < RAID_NODES; ++nodeId) {
            if (removeBlockFromNode(nodeId, blockId)) {
                cleanedCount++;
            }
        }
    }

    std::cout << "Limpiados " << cleanedCount << " bloques huérfanos\n";
    return cleanedCount;
}

std::unordered_map<std::string, std::vector<Raid5::FileNodeStatus>>
Raid5::getStatusPerFile() const {
    auto files = listFiles();
    auto global = getStatus();

    std::unordered_map<std::string, std::vector<FileNodeStatus>> result;
    result.reserve(files.size());

    for (const auto& fname : files) {
        std::vector<FileNodeStatus> vec;
        vec.reserve(global.size());
        for (const auto& gs : global) {
            FileNodeStatus fns;
            fns.nodeId   = gs.nodeId;
            fns.isActive = gs.isActive;
            vec.push_back(fns);
        }
        result.emplace(fname, std::move(vec));
    }

    return result;
}

// MÉTODOS INTERNOS
std::vector<uint8_t> Raid5::calculateParity(const std::vector<std::vector<uint8_t>>& bl) {
    if (bl.empty()) return {};
    size_t mx = 0;
    for (const auto& b : bl) mx = std::max(mx, b.size());
    std::vector<uint8_t> p(mx, 0);
    for (const auto& b : bl)
        for (size_t i = 0; i < b.size(); ++i)
            p[i] ^= b[i];
    return p;
}

bool Raid5::writeBlockToNode(int nodeId, int blockId, const std::vector<uint8_t>& d) {
    if (!isNodeActive(nodeId)) {
        std::cerr << "Error: Intento de escribir en nodo deshabilitado " << nodeId << "\n";
        return false;
    }
    
    std::string path = "node" + std::to_string(nodeId) +
                       "/block_" + std::to_string(blockId) + ".bin";
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<const char*>(d.data()), d.size());
    return true;
}

std::vector<uint8_t> Raid5::readBlockFromNode(int nodeId, int blockId) const {
    if (!isNodeActive(nodeId)) {
        return {}; // No leer de nodos deshabilitados
    }
    
    std::string path = "node" + std::to_string(nodeId) +
                       "/block_" + std::to_string(blockId) + ".bin";
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    return {std::istreambuf_iterator<char>(in), {}};
}

bool Raid5::isNodeActive(int nodeId) const {
    // Un nodo está activo si no está en la lista de deshabilitados
    return disabledNodes_.find(nodeId) == disabledNodes_.end();
}

int Raid5::getParityNode(int blockGroup) {
    return blockGroup % RAID_NODES;
}

std::vector<std::vector<uint8_t>> Raid5::splitIntoBlocks(const std::vector<uint8_t>& data) {
    std::vector<std::vector<uint8_t>> blocks;
    for (size_t i = 0; i < data.size(); i += BLOCK_SIZE) {
        size_t end = std::min(i + BLOCK_SIZE, data.size());
        blocks.emplace_back(data.begin() + i, data.begin() + end);
    }
    return blocks;
}

std::vector<uint8_t> Raid5::recoverBlock(int blockId, int failedNodeId) {
    // Implementación simplificada de recuperación
    // En una implementación real, se usaría XOR con otros bloques del grupo
    std::cerr << "Intento de recuperación de bloque " << blockId << " del nodo " << failedNodeId << "\n";
    return {};
}

// MÉTODOS AUXILIARES PARA LIMPIEZA
bool Raid5::removeBlockFromNode(int nodeId, int blockId) {
    std::string path = "node" + std::to_string(nodeId) +
                       "/block_" + std::to_string(blockId) + ".bin";
    
    if (std::filesystem::exists(path)) {
        try {
            return std::filesystem::remove(path);
        } catch (const std::exception& e) {
            std::cerr << "Error eliminando bloque " << blockId << " del nodo " << nodeId << ": " << e.what() << "\n";
            return false;
        }
    }
    
    return true; // El archivo no existe, consideramos que ya está "eliminado"
}

std::vector<int> Raid5::findOrphanedBlocks() const {
    std::unordered_set<int> validBlocks;
    
    // Recopilar todos los IDs de bloques válidos de los metadatos
    for (const auto& entry : fileMetadata_) {
        const auto& meta = entry.second;
        for (int blockId : meta.blockIds) {
            validBlocks.insert(blockId);
        }
        if (meta.parityBlockId >= 0) {
            validBlocks.insert(meta.parityBlockId);
        }
    }
    
    std::vector<int> orphanedBlocks;
    // Escanear todos los nodos en busca de archivos de bloques
    for (int nodeId = 0; nodeId < RAID_NODES; ++nodeId) {
        std::string nodeDir = "node" + std::to_string(nodeId);
        if (!std::filesystem::exists(nodeDir)) continue;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(nodeDir)) {
                if (!entry.is_regular_file()) continue;

                std::string filename = entry.path().filename().string();
                const std::string prefix = "block_";
                const std::string suffix = ".bin";

                // Comprueba que empiece con "block_" y termine con ".bin"
                if (filename.size() >= prefix.size() + suffix.size()
                    && filename.rfind(prefix, 0) == 0
                    && filename.compare(filename.size() - suffix.size(),
                                        suffix.size(), suffix) == 0)
                {
                    // Extraer la parte numérica entre prefijo y sufijo
                    std::string idStr = filename.substr(
                        prefix.size(),
                        filename.size() - prefix.size() - suffix.size()
                    );
                    try {
                        int blockId = std::stoi(idStr);
                        if (validBlocks.find(blockId) == validBlocks.end()) {
                            orphanedBlocks.push_back(blockId);
                        }
                    } catch (const std::exception&) {
                        // Ignorar archivos con nombres inválidos
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error escaneando nodo " << nodeId << ": " << e.what() << "\n";
        }
    }

    return orphanedBlocks;
}