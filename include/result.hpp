#ifndef SIFTER_RESULT_H
#define SIFTER_RESULT_H

#pragma once

#include <string>

#include <seqan3/search/dream_index/interleaved_bloom_filter.hpp>
#include <plog/Log.h>

#include <entry.hpp>

class Result
{
    private:
        uint8_t num_bins_{};
        std::unordered_map<std::string, ReadEntry> entries;

    public:
        Result() = default;
        Result(Result const &) = default;
        Result(Result &&) = default;
        Result & operator=(Result const &) = default;
        Result & operator=(Result &&) = default;
        ~Result() = default;

        Result(const uint8_t num_bins):
            num_bins_{num_bins}
        {};

        void update_entry(const std::string read_id, const uint16_t length, const auto & entry){
            /*std::cout << read_id << " ";
            for (const auto i : entry){
                std:: cout << +i;
            }
            std::cout << std::endl;*/
            if (entries.find(read_id) == entries.end()){
                PLOG_DEBUG << "Define entry for " << read_id << " with length " << length;
                entries[read_id] = ReadEntry(read_id, length, num_bins_);
            }
            PLOG_DEBUG << "Update entry";
            entries[read_id].update_entry(entry);
        };

        void print_result(const std::string read_id) {
            if (entries.find(read_id) != entries.end()) {
                entries[read_id].print_result();
            }
        };
    };

#endif // SIFTER_RESULT_H
