#pragma once
#include <string>
#include <vector>

struct DiskInfo {
    std::string ip;
    int port;
};

struct ControllerConfig {
    int controllerPort;
    std::vector<DiskInfo> disks;
};

ControllerConfig loadControllerConfig(const std::string& path);
