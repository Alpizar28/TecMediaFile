#include "Base64.h"
#include <string>
#include <vector>
#include <cstdint>

static const std::string chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string base64_encode(const std::string& in) {
    std::string out;
    int val = 0;
    int valb = -6;
    for (uint8_t c : reinterpret_cast<const std::string&>(in)) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}

std::string base64_decode(const std::string& in) {
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) {
        T[static_cast<uint8_t>(chars[i])] = i;
    }
    std::string out;
    int val = 0;
    int valb = -8;
    for (uint8_t c : reinterpret_cast<const std::string&>(in)) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}
