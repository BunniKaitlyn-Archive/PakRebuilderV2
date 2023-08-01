#pragma once
#include "app.h"
#include "sha1.h"
#include <fstream>
#include <cstdint>
#include <vector>

namespace pak_rebuilder {

class chunk_worker {
public:
    static int32_t file_count;
    static std::ofstream failed_log;
    int32_t worker_id = -1;
    std::vector<pak_rebuilder::pak_record*> pak_records{};

    chunk_worker(int32_t worker_id);

    void start();

private:
    void log_failed(std::string msg, bool new_line = false);
    std::string get_pak_dir_path(std::string path);
    std::string get_pak_filename(std::string path);
    bool validate_data(char* data, size_t data_length, char* hash);
    void extract_data(pak_rebuilder::pak_record* record);
};

}
