#include "Base64.h"
#include <algorithm>
#include <cstdint>
const std::string Base64::chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string Base64::encode(const std::vector<uint8_t>& data) {
    std::string result;
    int val = 0, valb = -6;
    
    for (uint8_t c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    while (result.size() % 4) {
        result.push_back('=');
    }
    
    return result;
}

std::vector<uint8_t> Base64::decode(const std::string& encoded) {
    std::vector<uint8_t> result;
    std::vector<int> T(128, -1);
    
    for (int i = 0; i < 64; i++) {
        T[chars[i]] = i;
    }
    
    int val = 0, valb = -8;
    for (char c : encoded) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    
    return result;
}

bool Base64::isBase64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}