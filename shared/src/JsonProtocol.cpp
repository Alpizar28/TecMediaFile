#include "JsonProtocol.h"

namespace JsonProtocol {

nlohmann::json WriteRequest::to_json() const {
    return nlohmann::json{
        {"blockId", blockId},
        {"data", data}
    };
}

WriteRequest WriteRequest::from_json(const nlohmann::json& j) {
    WriteRequest req;
    req.blockId = j.at("blockId").get<int>();
    req.data = j.at("data").get<std::string>();
    return req;
}

nlohmann::json ReadResponse::to_json() const {
    return nlohmann::json{
        {"blockId", blockId},
        {"data", data},
        {"success", success}
    };
}

ReadResponse ReadResponse::from_json(const nlohmann::json& j) {
    ReadResponse resp;
    resp.blockId = j.at("blockId").get<int>();
    resp.data = j.at("data").get<std::string>();
    resp.success = j.at("success").get<bool>();
    return resp;
}

nlohmann::json StatusResponse::to_json() const {
    return nlohmann::json{
        {"nodeId", nodeId},
        {"isActive", isActive},
        {"blocksUsed", blocksUsed},
        {"totalBlocks", totalBlocks}
    };
}

StatusResponse StatusResponse::from_json(const nlohmann::json& j) {
    StatusResponse status;
    status.nodeId = j.at("nodeId").get<int>();
    status.isActive = j.at("isActive").get<bool>();
    status.blocksUsed = j.at("blocksUsed").get<int>();
    status.totalBlocks = j.at("totalBlocks").get<int>();
    return status;
}

} // namespace JsonProtocol