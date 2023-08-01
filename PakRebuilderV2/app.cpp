#include "app.h"
#include "chunk_worker.h"
#include <thread>
#include <fstream>
#include <filesystem>

pak_rebuilder::app::app() {
    load_failed_records();
    load_pak_records();
    load_chunks();
}

void pak_rebuilder::app::start() {
    for (int32_t i = 0; i < NUM_WORKERS; i++) {
        int32_t current_index = i; // ptsd from C#, dunno if its required here
        std::thread thread([current_index]() { (new pak_rebuilder::chunk_worker(current_index))->start(); });
        thread.detach();
    }

    std::cin.get();
}

void pak_rebuilder::app::load_failed_records() {
    std::ifstream input("E:\\neo\\PakRebuilderV2\\x64\\Release\\failed_part1.log");
    for (std::string line; std::getline(input, line);) {
        failed_records.push_back(new pak_rebuilder::failed_record(line));
    }
}

void pak_rebuilder::app::load_pak_records() {
    int32_t index = 0;
    std::ifstream input("C:\\Users\\Kaitlyn\\source\\ConsoleApp1\\ConsoleApp1\\bin\\Debug\\net6.0\\FileList-OT11-Hash.txt");
    for (std::string line; std::getline(input, line);) {
        if (index == NUM_WORKERS) {
            index = 0;
        }
        pak_records[index].push_back(new pak_rebuilder::pak_record(line));
        index++;
        total_records++;
    }
}

void pak_rebuilder::app::load_chunks() {
    for (const auto& entry : std::filesystem::directory_iterator("D:\\Fortnite OT11 PC\\Fortnite OT11 PC\\Part1")) {
        std::ifstream file(entry.path(), std::ios::binary);
        file.unsetf(std::ios::skipws);
        file.seekg(0, std::ios::end);
        size_t length = file.tellg();
        file.seekg(0, std::ios::beg);
        char* buffer = static_cast<char*>(malloc(length));
        file.read(buffer, length);
        loaded_chunks.insert({entry.path().filename().string(), new pak_rebuilder::chunk_record(length, buffer)});
    }

    std::ifstream input("D:\\Fortnite OT11 PC\\Fortnite OT11 PC\\Part1.txt");
    for (std::string line; std::getline(input, line);) {
        chunk_tree.push_back(util::split(util::split(line, ',')[0], ' ')[1]);
    }
}
