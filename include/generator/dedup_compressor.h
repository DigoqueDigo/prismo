#ifndef DEDUP_COMPRESS_GENERATOR_H
#define DEDUP_COMPRESS_GENERATOR_H

#include <ranges>
#include <unordered_map>
#include <generator/synthetic.h>
#include <lib/distribution/percentage.h>
#include <boost/pool/pool.hpp>

#define DEDUP_WINDOW_SIZE 5

namespace Generator {

    struct DedupElement {
        uint64_t block_id;
        uint32_t left_repeats;
        uint32_t reduction;
        uint8_t* buffer;
    };

    struct DedupCompressorGeneratorConfig {
        // refill buffers flag;
        bool refill_buffers;
        // size of each buffer
        size_t buffer_size;
        // vector with dedup percentages to choose repeats
        std::vector<PercentageElement<uint32_t, uint32_t>> dedup_percentages;
        // vector with reduction percentages for each dedup model to choose reduction percentage
        std::unordered_map<uint32_t, std::vector<PercentageElement<uint32_t, uint32_t>>> reduction_percentages;

        void add_dedup_percentage(
            PercentageElement<uint32_t, uint32_t> dedup_percentage
        );

        void add_reduction_percentage(
            uint32_t repeats,
            PercentageElement<uint32_t, uint32_t> reduction_percentage
        );

        uint32_t select_repeats(uint32_t roll);

        uint32_t select_reduction(uint32_t roll, uint32_t repeats);

        void validate(void) const;
    };

    class DedupCompressorGenerator : public Generator {
        private:
            // buffer pool to allocate buffer for dedup elements
            boost::pool<> pool;
            // base buffer to use when reffil buffer flag is false
            std::unique_ptr<uint8_t[]> refill_buffer;
            // random_generator to refill buffers
            RandomGenerator random_generator;
            // config with all percentage parameters
            DedupCompressorGeneratorConfig config;
            // uniform distribution for select rolls
            Distribution::UniformDistribution<uint32_t> distribution;
            // map with sliding window for each dedup model
            std::unordered_map<uint32_t, std::vector<DedupElement>> dedup_windows;

            // return reused dedup element
            DedupElement reuse_dedup_element(uint32_t repeats, uint8_t* buffer, size_t size);

            // create dedup_element setting the buffer
            DedupElement create_dedup_element(uint32_t repeats, uint32_t reduction, size_t size);

        public:
            DedupCompressorGenerator() = delete;
            DedupCompressorGenerator(const DedupCompressorGeneratorConfig& _config);

            ~DedupCompressorGenerator() override {
                // std::cout << "~Destroying DedupCompressorGenerator" << std::endl;
            }

            BlockMetadata next_block(uint8_t* buffer, size_t size) override;
    };

    void from_json(const json& j, DedupCompressorGeneratorConfig& config);
}

#endif