// Raid5Helpers.cpp
#include "Raid5.h"
#include <algorithm>
#include <cstdint>

// Divide los datos en bloques de tamaño BLOCK_SIZE
std::vector<std::vector<uint8_t>> Raid5::splitIntoBlocks(const std::vector<uint8_t>& data) {
    std::vector<std::vector<uint8_t>> blocks;
    for (size_t off = 0; off < data.size(); off += BLOCK_SIZE) {
        size_t len = std::min(data.size() - off, BLOCK_SIZE);
        blocks.emplace_back(data.begin() + off, data.begin() + off + len);
    }
    return blocks;
}
// Determina el nodo de paridad para un grupo de bloques
int Raid5::getParityNode(int grp) {
    return grp % RAID_NODES;
}
// Repara un nodo completo reconstruyendo todos sus bloques
bool Raid5::recoverFromFailure(int failedNodeId) {
    if (failedNodeId < 0 || failedNodeId >= RAID_NODES) return false;
    bool ok = true;
    for (auto& kv : fileMetadata_) {
        const auto& meta = kv.second;
        // Bloques de datos
        for (int blockId : meta.blockIds) {
            auto recovered = recoverBlock(blockId, failedNodeId);
            if (!writeBlockToNode(failedNodeId, blockId, recovered)) {
                ok = false;
            }
        }
        // Bloque de paridad
        auto parity = recoverBlock(meta.parityBlockId, failedNodeId);
        if (!writeBlockToNode(failedNodeId, meta.parityBlockId, parity)) {
            ok = false;
        }
    }
    return ok;
}


// Reconstruye un bloque perdido usando los demás bloques y la paridad
std::vector<uint8_t> Raid5::recoverBlock(int bid, int /*fail*/) {
    std::vector<uint8_t> res(BLOCK_SIZE, 0);
    bool first = true;
    for (int n = 0; n < RAID_NODES; ++n) {
        auto blk = readBlockFromNode(n, bid);
        if (blk.empty()) continue;
        if (first) {
            res = blk;
            first = false;
        } else {
            for (size_t i = 0; i < blk.size() && i < res.size(); ++i)
                res[i] ^= blk[i];
        }
    }
    return res;
}