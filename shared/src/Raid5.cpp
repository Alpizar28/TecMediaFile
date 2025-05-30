// shared/src/Raid5.cpp (parte 1)
// Includes y singleton + initialize
#include "Raid5.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <ctime>
#include <cstdint>

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

// shared/src/Raid5.cpp (parte 2)
// Almacenar archivo en bloques y paridad
bool Raid5::storeFile(const std::string& filename, const std::vector<uint8_t>& data) {
    if (data.empty()) {
        std::cerr << "Error: datos vacíos para archivo " << filename << "\n";
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

        if (!writeBlockToNode(dataNode, blockId, blocks[i])) {
            std::cerr << "Error escribiendo bloque " << blockId << "\n";
            return false;
        }
        meta.blockIds.push_back(blockId);

        // Calcular y escribir bloque de paridad al final de cada grupo
        if ((i + 1) % (RAID_NODES - 1) == 0 || i + 1 == blocks.size()) {
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


// shared/src/Raid5.cpp (parte 3)
// Recuperar y eliminar archivos
std::vector<uint8_t> Raid5::retrieveFile(const std::string& filename) {
    auto it = fileMetadata_.find(filename);
    if (it == fileMetadata_.end()) return {};

    const auto& meta = it->second;
    std::vector<uint8_t> out;
    out.reserve(meta.fileSize);

    for (int bid : meta.blockIds) {
        bool ok = false;
        for (int node = 0; node < RAID_NODES && !ok; ++node) {
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
    fileMetadata_.erase(it);
    return true;
}

std::vector<std::string> Raid5::listFiles() {
    std::vector<std::string> names;
    names.reserve(fileMetadata_.size());
    for (auto& p : fileMetadata_) names.push_back(p.first);
    return names;
}


std::vector<NodeStatus> Raid5::getStatus() {
    std::vector<NodeStatus> st;
    st.reserve(RAID_NODES);
    for (int i = 0; i < RAID_NODES; ++i) {
        NodeStatus ns{i, isNodeActive(i), 0, 1000};
        for (auto& fm : fileMetadata_)
            for (int bid : fm.second.blockIds)
                if (bid % RAID_NODES == i) ns.blocksUsed++;
        st.push_back(ns);
    }
    return st;
}

bool Raid5::isSystemHealthy() {
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

size_t Raid5::getFileSize(const std::string& fn)  {
    auto it = fileMetadata_.find(fn);
    return it == fileMetadata_.end() ? 0 : it->second.fileSize;
}


bool Raid5::fileExists(const std::string& fn)  {
    return fileMetadata_.count(fn) > 0;
}

std::vector<uint8_t> Raid5::calculateParity(const std::vector<std::vector<uint8_t>>& bl) {
    if (bl.empty()) return {};
    size_t mx = 0;
    for (auto& b : bl) mx = std::max(mx, b.size());
    std::vector<uint8_t> p(mx, 0);
    for (auto& b : bl)
        for (size_t i = 0; i < b.size(); ++i)
            p[i] ^= b[i];
    return p;
}

// Escribe un bloque como fichero binario
bool Raid5::writeBlockToNode(int nodeId, int blockId, const std::vector<uint8_t>& d) {
    std::string path = "node" + std::to_string(nodeId) +
                       "/block_" + std::to_string(blockId) + ".bin";
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<const char*>(d.data()), d.size());
    return true;
}

// Lee un bloque de disco
std::vector<uint8_t> Raid5::readBlockFromNode(int nodeId, int blockId) {
    std::string path = "node" + std::to_string(nodeId) +
                       "/block_" + std::to_string(blockId) + ".bin";
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    return {std::istreambuf_iterator<char>(in), {}};
}


bool Raid5::isNodeActive(int)  {
    return true;
}