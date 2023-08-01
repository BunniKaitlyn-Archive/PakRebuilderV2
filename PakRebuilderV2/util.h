#pragma once
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string>

namespace pak_rebuilder {

class util {
public:
    static std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> result;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            result.push_back(item);
        }
        return result;
    }

    static int char2int(char input) {
        if (input >= '0' && input <= '9') {
            return input - '0';
        }
        if (input >= 'A' && input <= 'F') {
            return input - 'A' + 10;
        }
        if (input >= 'a' && input <= 'f') {
            return input - 'a' + 10;
        }
        throw std::invalid_argument("Invalid input string");
    }

    static void hex2bin(const char* src, char* target) {
        while (*src && src[1]) {
            *(target++) = char2int(*src) * 16 + char2int(src[1]);
            src += 2;
        }
    }

    static std::string concat_path(const std::string& first, const std::string& second) {
        std::string temp = first;
        if (first[first.length()] != '\\') {
            temp += '\\';
            return temp + second;
        } else {
            return first + second;
        }
    }
};

}
