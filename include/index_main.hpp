#ifndef CHARON_INDEX_MAIN_H
#define CHARON_INDEX_MAIN_H

#pragma once

#include <omp.h>
#include <cstring>

#include "CLI11.hpp"
#include <seqan3/search/dream_index/interleaved_bloom_filter.hpp>

class Index;
class InputStats;
class InputSummary;

/// Collection of all options of index subcommand.
struct IndexArguments {
    // IO options
    std::string input_file;
    std::string prefix;
    std::string tmp_dir;

    // kmer/sketching
    uint8_t window_size { 41 };
    uint8_t kmer_size { 19 };

    // IBF options
    mutable size_t bits {std::numeric_limits<uint32_t>::max()}; // Allow to change bits for each partition
    uint8_t num_hash {3};
    double max_fpr {0.01};

    // General options
    std::string log_file {"charon.log"};
    uint8_t threads { 1 };
    uint8_t verbosity { 0 };
    bool optimize { false };
};

void setup_index_subcommand(CLI::App& app);

InputSummary parse_input_file(const std::filesystem::path& input_file);

InputStats count_and_store_hashes(const IndexArguments& opt, const InputSummary& summary);

std::unordered_map<uint8_t, std::vector<uint8_t>> optimize_layout(const IndexArguments& arguments, const InputSummary& summary, const InputStats& stats);

Index build_index(const IndexArguments& opt, const InputSummary& summary, InputStats& stats);


int index_main(IndexArguments & opt);


#endif // CHARON_INDEX_MAIN_H
