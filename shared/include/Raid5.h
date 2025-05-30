#pragma once
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <ctime>

struct NodeStatus {
    int nodeId;
    bool isActive;
    int blocksUsed;
    int totalBlocks;
};

struct FileMetadata {
    std::string filename;
    size_t fileSize;
    std::vector<int> blockIds;
    int parityBlockId;
    std::time_t createdAt;
    std::time_t modifiedAt;
};

class Raid5 {
public:
    static Raid5& getInstance();
    
    // Configuración inicial
    bool initialize(const std::vector<std::string>& nodeUrls);
    
    // Operaciones de archivos - métodos principales
    bool storeFile(const std::string& filename, const std::vector<uint8_t>& data);
    std::vector<uint8_t> retrieveFile(const std::string& filename);
    std::vector<uint8_t> loadFile(const std::string& filename); // Alias para compatibilidad
    bool deleteFile(const std::string& filename);
    std::vector<std::string> listFiles();
    
    // Estado del sistema
    std::vector<NodeStatus> getStatus();
    bool isSystemHealthy();
    
    // Tolerancia a fallos
    bool recoverFromFailure(int failedNodeId);
    
    // Información adicional
    size_t getFileSize(const std::string& filename);
    bool fileExists(const std::string& filename);
    
private:
    Raid5() = default;
    ~Raid5() = default;
    Raid5(const Raid5&) = delete;
    Raid5& operator=(const Raid5&) = delete;
    
    static constexpr int RAID_NODES = 4;
    static constexpr size_t BLOCK_SIZE = 1024 * 1024; // p. ej. 1 MiB por bloque



    
    std::vector<std::string> nodeUrls_;
    std::map<std::string, FileMetadata> fileMetadata_;
    int nextBlockId_ = 0;
    
    // Métodos internos
    std::vector<uint8_t> calculateParity(const std::vector<std::vector<uint8_t>>& blocks);
    bool writeBlockToNode(int nodeId, int blockId, const std::vector<uint8_t>& data);
    std::vector<uint8_t> readBlockFromNode(int nodeId, int blockId);
    bool isNodeActive(int nodeId);
    int getParityNode(int blockGroup);
    std::vector<std::vector<uint8_t>> splitIntoBlocks(const std::vector<uint8_t>& data);
    std::vector<uint8_t> recoverBlock(int blockId, int failedNode);
};