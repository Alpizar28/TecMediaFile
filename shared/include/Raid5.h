// shared/include/Raid5.h
#pragma once

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <ctime>
#include <cstdint>
#include <unordered_set>

/// Estado global de un nodo de almacenamiento
struct NodeStatus {
    int     nodeId;       // identificador del nodo (0..N-1)
    bool    isActive;     // si el nodo está activo
    int     blocksUsed;   // número de bloques ocupados
    int     totalBlocks;  // capacidad total de bloques
};

/// Metadatos de cada archivo en el sistema RAID
struct FileMetadata {
    std::string          filename;
    size_t               fileSize;
    std::vector<int>     blockIds;
    int                  parityBlockId;
    std::time_t          createdAt;
    std::time_t          modifiedAt;
};

class Raid5 {
public:
    static Raid5& getInstance();

    // Inicialización
    bool initialize(const std::vector<std::string>& nodeUrls);

    // Operaciones sobre archivos
    bool storeFile(const std::string& filename, const std::vector<uint8_t>& data);
    std::vector<uint8_t> retrieveFile(const std::string& filename);
    std::vector<uint8_t> loadFile(const std::string& filename);
    bool deleteFile(const std::string& filename);
    std::vector<std::string> listFiles() const;

    // Estado global del sistema
    std::vector<NodeStatus> getStatus() const;
    bool isSystemHealthy() const;

    // Tolerancia a fallos
    bool recoverFromFailure(int failedNodeId);

    // Información adicional
    size_t getFileSize(const std::string& filename) const;
    bool fileExists(const std::string& filename) const;

    // Métodos de limpieza (NUEVOS)
    bool cleanupDeletedFile(const std::string& filename);
    int cleanupOrphanedNodes();

    /// Estado de un nodo específico para cada archivo
    struct FileNodeStatus {
        int   nodeId;
        bool  isActive;
    };

    /**
     * @brief Mapea cada archivo a un vector de FileNodeStatus
     * @return unordered_map<string filename, vector<FileNodeStatus>>
     */
    std::unordered_map<std::string, std::vector<FileNodeStatus>> getStatusPerFile() const;
    bool disableNode(int nodeId);
    bool enableNode(int nodeId);
    std::vector<int> getDisabledNodes() const;

private:
    Raid5() = default;
    ~Raid5() = default;
    Raid5(const Raid5&) = delete;
    Raid5& operator=(const Raid5&) = delete;

    static constexpr int RAID_NODES = 4;
    static constexpr size_t BLOCK_SIZE = 1024 * 1024;

    std::vector<std::string>            nodeUrls_;
    std::map<std::string, FileMetadata> fileMetadata_;
    int                                  nextBlockId_ = 0;

    // Métodos internos
    std::vector<uint8_t> calculateParity(const std::vector<std::vector<uint8_t>>& blocks);
    bool writeBlockToNode(int nodeId, int blockId, const std::vector<uint8_t>& data);
    std::vector<uint8_t> readBlockFromNode(int nodeId, int blockId) const;
    bool isNodeActive(int nodeId) const;
    int getParityNode(int blockGroup);
    std::vector<std::vector<uint8_t>> splitIntoBlocks(const std::vector<uint8_t>& data);
    std::vector<uint8_t> recoverBlock(int blockId, int failedNodeId);
    
    // Métodos auxiliares para limpieza (NUEVOS)
    bool removeBlockFromNode(int nodeId, int blockId);
    std::vector<int> findOrphanedBlocks() const;

    std::unordered_set<int> disabledNodes_;

};