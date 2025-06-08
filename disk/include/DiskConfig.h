#pragma once
#include <string>

struct DiskConfig {
    std::string ip;
    int port;
    std::string path;
    size_t size;
    size_t blockSize;
};

DiskConfig loadDiskConfig(const std::string& path);
