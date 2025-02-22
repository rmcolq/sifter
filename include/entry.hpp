#ifndef CHARON_ENTRY_H
#define CHARON_ENTRY_H

#pragma once

#include <string>

#include <plog/Log.h>

#include <counts.hpp>
#include <input_summary.hpp>
#include <classify_stats.hpp>


class ReadEntry {
private:
    std::string read_id_;
    uint32_t length_;
    uint32_t num_hashes_{0};
    std::unordered_map<uint8_t, std::vector<bool>> bits_; // this collects over all bins
    std::unordered_map<uint8_t, std::vector<bool>> max_bits_; // this summarizes over categories (which may have multiple bins)
    Counts<uint32_t> counts_;
    std::vector<float> props_; // this collects over categories the proportion of all hashes which were from the given category
    //std::vector<float> unique_props_; // this collects over categories the proportion of all hashes which were unique to the given category
    std::vector<double> probabilities_; // this collects over categories the probability of this read given the data come from that category
    uint8_t call_ = std::numeric_limits<uint8_t>::max();
    int8_t confidence_score_ = std::numeric_limits<int8_t>::max();
public:
    ReadEntry() = default;

    ReadEntry(ReadEntry const &) = default;

    ReadEntry(ReadEntry &&) = default;

    ReadEntry &operator=(ReadEntry const &) = default;

    ReadEntry &operator=(ReadEntry &&) = default;

    ~ReadEntry() = default;

    ReadEntry(const std::string read_id, const uint32_t length, const InputSummary &summary) :
            read_id_(read_id),
            length_(length) {
        PLOG_DEBUG << "Initialize entry with read_id " << read_id << " and length " << length;
        counts_.set_size(summary.num_categories());
        for (auto i = 0; i < summary.num_bins; ++i) {
            bits_[i].reserve(length);
        }
        for (auto i = 0; i < summary.num_categories(); ++i) {
            max_bits_[i].reserve(length);
            props_.emplace_back(0);
            //unique_props_.emplace_back(0);
            probabilities_.emplace_back(1);
        }
        PLOG_VERBOSE << "Initializing complete for read_id " << read_id;
    }

    std::vector<float> props() const {
        return props_;
    }

    /*std::vector<float> unique_props() const {
        return unique_props_;
    }*/

    uint8_t call() const {
        return call_;
    }

    uint8_t confidence_score() const {
        return confidence_score_;
    }

    void update_entry(const auto &entry) {
        // this "entry" is a bitvector with a 1 or 0 for each bin in the ibf
        /*PLOG_DEBUG << " entry [";
        for (const auto i : entry){
            PLOG_DEBUG << i;
        }
        PLOG_DEBUG << "]" ;*/

        assert(entry.size() == bits_.size());
        num_hashes_ += 1;
        for (auto bucket = 0; bucket < entry.size(); ++bucket) {
            bits_.at(bucket).emplace_back(entry[bucket]);
            if (bits_.at(bucket).size() != num_hashes_){
#pragma omp critical
                PLOG_ERROR << read_id_ << " bucket k=" << +bucket << " out of " << entry.size() << " final bin/hash sizes " << bits_.at(bucket).size() << " and " << num_hashes_;
            }
            assert(bits_.at(bucket).size() == num_hashes_);
        }
    };

    void get_max_bits(const InputSummary &summary) {
        PLOG_DEBUG << "Get max bits per category for read " << read_id_;
        const auto num_categories = max_bits_.size();
        for (auto i = 0; i < num_categories; ++i) {
            max_bits_.at(i).resize(num_hashes_, 0);
        }
        for (const auto &[bin, bitmap]: bits_) {
            assert(bitmap.size() == num_hashes_);
            const auto & category = summary.bin_to_category.at(bin);
            const auto & index = summary.category_index(category);
            assert(index < num_categories);
            PLOG_VERBOSE << +bin << " belongs to " << category << " with index " << +index << " for read " << read_id_;

            auto current_bit_count = std::count(bitmap.begin(), bitmap.end(), true);
            const auto &max_bitmap = max_bits_.at(index);
            auto max_bit_count = std::count(max_bitmap.begin(), max_bitmap.end(), true);
            PLOG_VERBOSE << current_bit_count << " " << max_bit_count << " for read " << read_id_;
            if (current_bit_count > max_bit_count) {
                PLOG_VERBOSE << "redefine max for read " << read_id_;
                max_bits_.at(index) = bitmap;
            }
        }
    };

    void get_counts() {
        PLOG_DEBUG << "Collect_counts for read_id " << read_id_;
        const auto num_categories = max_bits_.size();
        for (auto i = 0; i < num_categories; ++i) {
            for (auto j = 0; j <= i; ++j) {
                const auto & row = max_bits_.at(i);
                const auto & col = max_bits_.at(j);
                assert(row.size() == num_hashes_ and col.size() == num_hashes_);
                for (auto k = 0; k < num_hashes_; ++k) {
                    //PLOG_DEBUG << "(" << i << "," << j << ")" << k << row.size() << col.size();
                    if (row.at(k) and col.at(k)) {
                        counts_(i, j) += 1;
                    }
                }
                //PLOG_DEBUG << "done hashes";
            }
            //PLOG_DEBUG << "done category " << i;
        }
        //PLOG_DEBUG << "done";
    };

    void get_props() {
        PLOG_DEBUG << "collect props for read_id " << read_id_;
        const auto num_categories = props_.size();
        for (auto i = 0; i < num_categories; ++i) {
            props_.at(i) = static_cast< float >(counts_(i, i)) / static_cast< float >(num_hashes_);
        }
        return;
    };

    /*void get_unique_props() {
        PLOG_DEBUG << "collect_unique_props for read_id " << read_id_;
        const auto num_categories = unique_props_.size();
        assert(max_bits_.size() == num_categories);
        std::vector<uint32_t> unique_counts(num_categories, 0);
        std::vector<uint8_t> found;
        for (auto k = 0; k < num_hashes_; ++k) {
            found.clear();
            for (auto i = 0; i < num_categories; ++i) {
                const auto & row = max_bits_.at(i);
                if (k > row.size())
                    PLOG_ERROR << "k > row.size()";
                if (row.at(k)) {
                    found.push_back(i);
                }
            }
            if (found.size() == 1) {
                auto i = found.front();
                unique_counts.at(i) += 1;
            }
        }
        for (auto i = 0; i < num_categories; ++i) {
            unique_props_.at(i) = static_cast< float >(unique_counts.at(i)) / static_cast< float >(num_hashes_);
        }
        return;
    };*/

    void post_process(const InputSummary &summary) {
        get_max_bits(summary);
        get_counts();
        //get_unique_props();
        get_props();
    }

    void call_category(int8_t confidence_threshold, uint8_t min_num_hits) {
        for (auto i=0; i<probabilities_.size(); ++i)
        {
            if (counts_(i,i) > min_num_hits){
                continue;
            }
            return; // if none of the categories has at least the min_num_hits, no call
        }

        double first = 0;
        uint8_t first_pos = 0;
        double second = 0;
        uint8_t second_pos = 0;

        for (auto i = 0; i < probabilities_.size(); ++i) {
            const double &val = probabilities_.at(i);
            if (val > second) {
                second = val;
                second_pos = i;
                if (second > first)
                {
                    std::swap(first, second);
                    std::swap(first_pos, second_pos);
                }
            }
        }
        if (second == 0 and first > 0)
            call_ = first_pos;
        else
        {
            confidence_score_ = static_cast<int8_t>(std::log10(first/second));
            if (confidence_score_ > confidence_threshold)
                call_ = first_pos;
        }
    }

    void classify(const StatsModel &stats_model) {
        PLOG_DEBUG << "Classify read " << read_id_;;
        for (auto i = 0; i < props_.size(); ++i) {
            const auto &read_proportion = props_.at(i);
            const auto result_pair = stats_model.classify(i, read_proportion);
            PLOG_DEBUG << "Pos " << +i << " has read proportion " << read_proportion << " yielding probs "
                       << result_pair.pos << " and " << result_pair.neg << " for read " << read_id_;;
            for (auto j = 0; j < props_.size(); ++j) {
                if (i == j)
                    probabilities_.at(j) *= result_pair.pos;
                else
                    probabilities_.at(j) *= result_pair.neg;
            }
        }
        call_category(stats_model.confidence_threshold(), stats_model.min_num_hits());
    }

    void print_result(const InputSummary &summary) {
        std::cout << read_id_ << "\t" << num_hashes_ << "\t" << summary.category_name(call_) << "\t" << +confidence_score_ << "\t" ;
        for (auto i = 0; i < summary.num_categories(); i++) {
            std::cout << summary.categories.at(i) << ":" << counts_(i, i) << ":" << probabilities_.at(i) << "\t";
        }
        for (auto i = 0; i < summary.num_categories(); i++) {
            for (auto j = 0; j < i; j++) {
                std::cout << summary.categories.at(i) << "_x_" << summary.categories.at(j) << ":" << counts_(i, j)
                          << "\t";
            }
        }
        /*for (auto i=0; i<bits_.size(); i++){
            for (auto j=0; j<bits_.at(i).size(); j++) {
                std::cout << +bits_.at(i)[j];
            }
            std::cout << "\t";
        }*/
        for (auto i = 0; i < summary.num_categories(); i++) {
            //const auto & category = summary.categories.at(i);
            const auto &bitvector = max_bits_.at(i);
            for (auto j = 0; j < bitvector.size(); j++) {
                std::cout << +bitvector.at(j);
            }
            std::cout << "\t";
        }
        std::cout << std::endl;
    };

    void print_assignment_result(const InputSummary &summary) {
        // mimic the kraken assignment format with tab separated columns classification status, read_id call, num_hashes, details
        if (call_ == std::numeric_limits<uint8_t>::max())
            std::cout << "U" << "\t";
        else
            std::cout << "C" << "\t";
        std::cout << read_id_ << "\t" << summary.category_name(call_) << "\t" << num_hashes_ << "\t" << +confidence_score_ << "\t" ;
        std::cout.precision(6);
        for (auto i = 0; i < summary.num_categories(); i++) {
            std::cout << summary.categories.at(i) << ":" << counts_(i, i) << ":" << props_.at(i) << ":"
                      << probabilities_.at(i) << " ";
        }

        std::cout << std::endl;
    };
};

#endif // CHARON_ENTRY_H
