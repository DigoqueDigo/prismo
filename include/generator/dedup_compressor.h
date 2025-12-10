#ifndef DEDUP_COMPRESS_GENERATOR_H
#define DEDUP_COMPRESS_GENERATOR_H

#include <ranges>
#include <unordered_map>
#include <generator/synthetic.h>

namespace Generator {

    template<typename PercentageT, typename ValueT>
    struct PercentageElement {
        PercentageT cumulative_percentage;
        ValueT value;
    };

    template <typename PercentageT, typename ValueT>
    inline ValueT select_from_distribution(
        PercentageT roll,
        const std::vector<PercentageElement<PercentageT, ValueT>>& items
    ) {
        for (const auto& item : items) {
            if (roll < item.cumulative_percentage) {
                return item.value;
            }
        }
        throw std::runtime_error("select_from_distribution: roll out of range");
    }

    struct DedupElement {
        uint64_t block_id;
        uint32_t left_repeats;
    };

    class DedupCompressorGenerator : public Generator {
        private:
            size_t dedup_window_size = 3;
            Distribution::UniformDistribution<uint32_t> distribution;

            std::vector<PercentageElement<uint32_t, uint32_t>> dedup_percentages;
            std::unordered_map<uint32_t, std::vector<DedupElement>> models_dedup; // a key é o repeats
            std::unordered_map<uint32_t, std::shared_ptr<uint8_t[]>> models_base_buffer; // a key é o repeats
            std::unordered_map<uint32_t, std::vector<PercentageElement<uint32_t, uint32_t>>> models_reduction_percentage; // a key é o repeats

        public:
            DedupCompressorGenerator();

            ~DedupCompressorGenerator() override {
                // std::cout << "~Destroying DedupCompressorGenerator" << std::endl;
            }

            void add_dedup_percentage(uint32_t repeats, PercentageElement<uint32_t, uint32_t> dedup_percentage);
            void add_reduction_percentage(uint32_t repeats, PercentageElement<uint32_t, uint32_t> reduction_percentage);
            void set_model_base_buffer(uint32_t repeats, std::shared_ptr<uint8_t[]>& buffer);

            void validate(void);

            uint64_t nextBlock(uint8_t* buffer, size_t size) override;
            uint64_t applyCompression(uint8_t* buffer, size_t size);

            friend void from_json(const json& j, DedupCompressorGenerator& generator);
    };

    void from_json(const json& j, DedupCompressorGenerator& generator);
}

#endif