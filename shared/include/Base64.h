#pragma once
#include <string>
#include <vector>
#include <cstdint>

class Base64 {
public:
    static std::string encode(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> decode(const std::string& encoded);
    
private:
    static const std::string chars;
    static bool isBase64(unsigned char c);
};