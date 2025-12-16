#ifndef DEDUP_COMPRESS_GENERATOR_H
#define DEDUP_COMPRESS_GENERATOR_H

#include <ranges>
#include <unordered_map>
#include <generator/synthetic.h>
#include <lib/distribution/percentage.h>

namespace Generator {

    struct DedupElement {
        uint64_t block_id;
        uint32_t left_repeats;
    };

    class DedupCompressorGenerator : public Generator {
        private:
            size_t dedup_window_size = 3;
            Distribution::UniformDistribution<uint32_t> distribution;

            // vector with dedup percentages to choose repeats
            std::vector<PercentageElement<uint32_t, uint32_t>> dedup_percentages;

            // map with sliding window for each dedup model
            std::unordered_map<uint32_t, std::vector<DedupElement>> models_dedup;

            // random base buffer for each dedup model
            std::unordered_map<uint32_t, std::shared_ptr<uint8_t[]>> models_base_buffer;

            // vector with reduction percentages for each dedup model to choose reduction percentage
            std::unordered_map<uint32_t, std::vector<PercentageElement<uint32_t, uint32_t>>> models_reduction_percentage;

        public:
            DedupCompressorGenerator();

            ~DedupCompressorGenerator() override {
                // std::cout << "~Destroying DedupCompressorGenerator" << std::endl;
            }

            void add_dedup_percentage(uint32_t repeats, PercentageElement<uint32_t, uint32_t> dedup_percentage);
            void add_reduction_percentage(uint32_t repeats, PercentageElement<uint32_t, uint32_t> reduction_percentage);
            void set_model_base_buffer(uint32_t repeats, std::shared_ptr<uint8_t[]>& buffer);

            void validate(void) const;

            BlockMetadata next_block(uint8_t* buffer, size_t size) override;
            friend void from_json(const json& j, DedupCompressorGenerator& generator);
    };

    void from_json(const json& j, DedupCompressorGenerator& generator);
}

#endif