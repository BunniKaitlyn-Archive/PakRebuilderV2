#include "chunk_worker.h"
#include "app.h"
#include <algorithm>
#include <functional>
#include <filesystem>

int32_t pak_rebuilder::chunk_worker::file_count = 0;
std::ofstream pak_rebuilder::chunk_worker::failed_log = std::ofstream("E:\\neo\\PakRebuilderV2\\x64\\Release\\failed.log");

pak_rebuilder::chunk_worker::chunk_worker(int32_t worker_id) {
    this->worker_id = worker_id;
    this->pak_records = g_App->pak_records[worker_id];
}

void pak_rebuilder::chunk_worker::start() {
    printf("[W%i] Starting. Records = %llu\n", worker_id, pak_records.size());
    for (pak_rebuilder::pak_record* record : pak_records) {
        extract_data(record);
    }
    printf("[W%i] Finished!\n", worker_id);
}

void pak_rebuilder::chunk_worker::log_failed(std::string msg, bool new_line) {
    if (failed_log.is_open()) {
        if (new_line) {
            failed_log << msg << "\n";
        } else {
            failed_log << msg;
        }
        failed_log.flush();
    }
}

std::string pak_rebuilder::chunk_worker::get_pak_dir_path(std::string path) {
    auto ss = util::split(path, '/');
    ss.pop_back();
    std::ostringstream result;
    std::copy(ss.begin(), ss.end(), std::ostream_iterator<std::string>(result, "\\"));
    std::string value = result.str();
    return value.substr(0, value.size() - 1);;
}

std::string pak_rebuilder::chunk_worker::get_pak_filename(std::string path) {
    auto ss = util::split(path, '/');
    return ss[ss.size() - 1];
}

bool pak_rebuilder::chunk_worker::validate_data(char* data, size_t data_length, char* hash) {
    char result[20];
    SHA1(result, data, static_cast<int32_t>(data_length));
    return memcmp(result, hash, 20) == 0;
}

void pak_rebuilder::chunk_worker::extract_data(pak_rebuilder::pak_record* record) {
    std::string dir_path = util::concat_path("D:\\Fortnite OT11 PC\\Fortnite OT11 PC\\Dumped_Failed", get_pak_dir_path(record->file_name));
    std::string file_path = util::concat_path(dir_path, get_pak_filename(record->file_name));

    bool file_exists = true;
    if (file_exists) {
        // make sure file also wasn't failed.
        for (int32_t idx = 0; idx < g_App->failed_records.size(); idx++) {
            pak_rebuilder::failed_record* failed_record = g_App->failed_records[idx];
            if (record->file_name.compare(failed_record->file_name) == 0) {
                file_exists = false;
                printf("[W%i] Ignoring failed file %s! (%i/%i)\n", worker_id, record->file_name.c_str(), file_count, g_App->total_records);
                break;
            }
        }
    }

    if (!file_exists) {
        for (auto i : g_App->loaded_chunks) {
            pak_rebuilder::chunk_record* chunk_record = i.second;

            char* it = std::search(chunk_record->buffer, chunk_record->buffer + chunk_record->length, std::boyer_moore_horspool_searcher(record->data_hash, record->data_hash + 20));
            if (it != chunk_record->buffer + chunk_record->length) {
                char* pak_data = it + 25;

                if (pak_data + record->data_size < chunk_record->buffer + chunk_record->length) {
                    if (/*validate_data(pak_data, record->data_size, record->data_hash)*/ true) {
                        file_count++;
                        std::filesystem::create_directories(dir_path);
                        std::fstream file;
                        file.open(file_path, std::ios::app | std::ios::binary);
                        file.write(pak_data, record->data_size);
                        file.flush();
                        file.close();
                        printf("[W%i] Needle found in %s for file %s! (%i/%i)\n", worker_id, i.first.c_str(), record->file_name.c_str(), file_count, g_App->total_records);
                    } else {
                        char buffer[1024];
                        snprintf(buffer, 1024, "HASH_INVALID|%s|%s|%i:%i\n", i.first.c_str(), record->file_name.c_str(), file_count, g_App->total_records);
                        log_failed(buffer);
                        printf("[W%i] Hash is invalid in %s for file %s! (%i/%i)\n", worker_id, i.first.c_str(), record->file_name.c_str(), file_count, g_App->total_records);
                    }
                } else {
                    auto itr = std::find(g_App->chunk_tree.begin(), g_App->chunk_tree.end(), i.first);
                    if (itr != g_App->chunk_tree.end()) {
                        ptrdiff_t chunk_index = std::distance(g_App->chunk_tree.begin(), itr);
                        ptrdiff_t pak_data_offset = std::distance(chunk_record->buffer, pak_data);

                        size_t new_chunk_length = chunk_record->length;
                        char* new_chunk_data = static_cast<char*>(malloc(new_chunk_length));

                        memcpy_s(new_chunk_data, chunk_record->length, chunk_record->buffer, chunk_record->length);

                        for (int32_t j = 1; j < 10; j++) {
                            if (static_cast<size_t>(chunk_index + j) > g_App->chunk_tree.size() - 1) {
                                break;
                            }

                            std::string next_chunk_file = g_App->chunk_tree.at(chunk_index + j);
                            pak_rebuilder::chunk_record* next_chunk_record = g_App->loaded_chunks[next_chunk_file];

                            char* realloc_chunk_data = static_cast<char*>(realloc(new_chunk_data, new_chunk_length + next_chunk_record->length));
                            new_chunk_data = realloc_chunk_data;
                            memcpy_s(new_chunk_data + new_chunk_length, next_chunk_record->length, next_chunk_record->buffer, next_chunk_record->length);
                            char* new_pak_data = new_chunk_data + pak_data_offset;

                            new_chunk_length += next_chunk_record->length;

                            if (new_pak_data + record->data_size < new_chunk_data + new_chunk_length) {
                                if (/*validate_data(new_pak_data, record->data_size, record->data_hash)*/true) {
                                    file_count++;
                                    std::filesystem::create_directories(dir_path);
                                    std::fstream file;
                                    file.open(file_path, std::ios::app | std::ios::binary);
                                    file.write(new_pak_data, record->data_size);
                                    file.flush();
                                    file.close();
                                    printf("[W%i] Needle found in multiple chunks (starting with %s) for file %s! (%i/%i)\n", worker_id, i.first.c_str(), record->file_name.c_str(), file_count, g_App->total_records);
                                    break;
                                } else {
                                    char buffer[1024];
                                    snprintf(buffer, 1024, "HASH_INVALID_MULTIPLE_CHUNKS|%s|%s|%i:%i\n", i.first.c_str(), record->file_name.c_str(), file_count, g_App->total_records);
                                    log_failed(buffer);
                                    printf("[W%i] Hash is invalid in multiple chunks (starting with %s) for file %s! (%i/%i)\n", worker_id, i.first.c_str(), record->file_name.c_str(), file_count, g_App->total_records);
                                    break;
                                }
                            }
                        }

                        free(new_chunk_data);
                    }
                }
                break;
            }
        }
    } else {
        file_count++;
    }
}
