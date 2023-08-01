#pragma once
#include "util.h"
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

#define NUM_WORKERS 15

namespace pak_rebuilder {

struct pak_record {
public:
    std::string file_name;
    char data_hash[20];
    uint64_t data_size;

    pak_record(std::string s) {
        auto ss = util::split(s, '|');
        this->file_name = ss[0];
        util::hex2bin(ss[1].c_str(), this->data_hash);
        this->data_size = std::stoul(ss[3], nullptr, 16);
    }
};

struct chunk_record {
public:
    size_t length;
    char* buffer;

    chunk_record(size_t length, char* buffer) {
        this->length = length;
        this->buffer = buffer;
    }
};

struct failed_record {
public:
    std::string reason;
    std::string starting_chunk;
    std::string file_name;

    failed_record(std::string s) {
        auto ss = util::split(s, '|');
        this->reason = ss[0];
        this->starting_chunk = ss[1];
        this->file_name = ss[2];
    }
};

class app {
public:
    int32_t exit_code = 0;
    int32_t total_records = 0;
    std::vector<pak_rebuilder::pak_record*> pak_records[NUM_WORKERS]{};
    std::unordered_map<std::string, pak_rebuilder::chunk_record*> loaded_chunks{};
    std::vector<std::string> chunk_tree{}; // maintains order of chunks, etc.

    std::vector<pak_rebuilder::failed_record*> failed_records{};

    app();

    void start();

private:
    void load_failed_records();
    void load_pak_records();
    void load_chunks();
};

}

extern pak_rebuilder::app* g_App;
