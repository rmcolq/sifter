#ifndef SIFTER_INDEX_MAIN_H
#define SIFTER_INDEX_MAIN_H

#include <omp.h>
#include <cstring>

#include "CLI11.hpp"
#include <seqan3/search/dream_index/interleaved_bloom_filter.hpp>

#include "index.hpp"

/// Collection of all options of index subcommand.
struct IndexOptions {
    std::string input_file;
    uint8_t window_size { 31 };
    uint8_t kmer_size { 15 };
    double fpr {0.05};
    std::string prefix;
    uint8_t threads { 1 };
    uint8_t verbosity { 0 };
};

struct Input;
struct InputOptions;

struct Input {
    std::unordered_map<std::string, uint8_t> filepath_to_bin;
    std::unordered_map<uint8_t, std::string> bin_to_name;
};

void setup_index_subcommand(CLI::App& app);
int build_index(IndexOptions const& opt);

Input parse_input_file(const std::filesystem::path& input_file);

seqan3::interleaved_bloom_filter<seqan3::data_layout::compressed> construct_ibf(const Input& , const uint8_t& , const uint8_t& );

#endif // SIFTER_INDEX_MAIN_H
