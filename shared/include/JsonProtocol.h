#pragma once
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

namespace JsonProtocol {
    
struct WriteRequest {
    int blockId;
    std::string data; // base64 encoded
    
    nlohmann::json to_json() const;
    static WriteRequest from_json(const nlohmann::json& j);
};

struct ReadResponse {
    int blockId;
    std::string data; // base64 encoded
    bool success;
    
    nlohmann::json to_json() const;
    static ReadResponse from_json(const nlohmann::json& j);
};

struct StatusResponse {
    int nodeId;
    bool isActive;
    int blocksUsed;
    int totalBlocks;
    
    nlohmann::json to_json() const;
    static StatusResponse from_json(const nlohmann::json& j);
};

} // namespace JsonProtocol