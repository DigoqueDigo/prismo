#include <generator/dedup_compressor.h>

namespace Generator {

    DedupCompressorGenerator::DedupCompressorGenerator()
        : Generator(), distribution(0, 99),
        dedup_percentages(), models_dedup(),
        models_base_buffer(), models_reduction_percentage() {}

    BlockMetadata DedupCompressorGenerator::next_block(uint8_t* buffer, size_t size) {
        uint32_t roll = distribution.nextValue();
        uint32_t selected_repeats = select_from_percentage_vector(roll, dedup_percentages);
        uint32_t selected_reduction = select_from_percentage_vector(
            roll, models_reduction_percentage[selected_repeats]);

        uint32_t bytes_reduction = size * selected_reduction / 100;
        std::shared_ptr<uint8_t[]>& base_buffer = models_base_buffer[selected_repeats];

        std::memset(buffer, 0, bytes_reduction);
        std::memcpy(buffer + bytes_reduction, base_buffer.get(), size - bytes_reduction);

        std::vector<DedupElement>& dedup_window = models_dedup[selected_repeats];

        if (selected_repeats == 0) {
            std::memcpy(buffer, &block_id, sizeof(block_id));
            return BlockMetadata {
                .block_id = block_id++,
                .compression = selected_reduction
            };
        }

        if (dedup_window.size() == dedup_window_size) {
            uint32_t index = distribution.nextValue() % dedup_window_size;
            DedupElement& element = dedup_window[index];

            std::memcpy(buffer, &element.block_id, sizeof(element.block_id));
            element.left_repeats--;

            if (element.left_repeats == 0) {
                std::swap(dedup_window[index], dedup_window.back());
                dedup_window.pop_back();
            }

            return BlockMetadata {
                .block_id = element.block_id,
                .compression = selected_reduction
            };
        }

        DedupElement element = {
            .block_id = block_id++,
            .left_repeats = selected_repeats,
        };

        dedup_window.push_back(element);
        std::memcpy(buffer, &(element.block_id), sizeof(element.block_id));

        return BlockMetadata {
            .block_id = element.block_id,
            .compression = selected_reduction
        };
    }

    void DedupCompressorGenerator::add_dedup_percentage(
        uint32_t repeats,
        PercentageElement<uint32_t, uint32_t> dedup_percentage
    ) {
        auto it = models_dedup.find(repeats);
        if (it != models_dedup.end()) {
            throw std::invalid_argument("add_dedup_percentage: percentage already registered for repeats: " + std::to_string(repeats));
        }
        models_dedup.try_emplace(repeats);
        dedup_percentages.push_back(dedup_percentage);
    }

    void DedupCompressorGenerator::set_model_base_buffer(
        uint32_t repeats,
        std::shared_ptr<uint8_t[]>& buffer
    ) {
        models_base_buffer[repeats] = std::move(buffer);
    }

    void DedupCompressorGenerator::add_reduction_percentage(
        uint32_t repeats,
        PercentageElement<uint32_t, uint32_t> reduction_percentage
    ) {
        auto& reduction_percentages = models_reduction_percentage[repeats];
        reduction_percentages.push_back(reduction_percentage);
    }

    void DedupCompressorGenerator::validate() const {
        validate_percentage_vector(dedup_percentages, "dedup");
        for (const auto& [key, vec] : models_reduction_percentage) {
            validate_percentage_vector(vec, "reduction for repeats " + std::to_string(key));

            bool has_invalid_reduction = std::any_of(vec.begin(), vec.end(),
                [](const auto& elem){ return elem.value > 100; });

            if (has_invalid_reduction) {
                throw std::invalid_argument("validate: reduction value must be less than 100");
            }
        }
    }

    void from_json(const json& j, DedupCompressorGenerator& generator) {
        uint32_t dedup_cumulative = 0;
        uint32_t reduction_cumulative = 0;

        RandomGenerator random_generator;
        size_t block_size = j.at("block_size").get<size_t>();

        for (const auto& dedup_item : j.at("distribution")) {
            uint32_t repeats = dedup_item.at("repeats").get<uint32_t>();
            dedup_cumulative += dedup_item.at("percentage").get<uint32_t>();

            PercentageElement<uint32_t, uint32_t> dedup_percentage {
                .cumulative_percentage = dedup_cumulative,
                .value = repeats,
            };

            auto base_buffer = std::make_shared<uint8_t[]>(block_size);
            random_generator.next_block(base_buffer.get(), block_size);

            generator.add_dedup_percentage(repeats, dedup_percentage);
            generator.set_model_base_buffer(repeats, base_buffer);

            reduction_cumulative = 0;

            for (const auto& reduction_item : dedup_item.at("compression")) {
                reduction_cumulative += reduction_item.at("percentage").get<uint32_t>();

                PercentageElement<uint32_t, uint32_t> reduction_percentage {
                    .cumulative_percentage = reduction_cumulative,
                    .value = reduction_item.at("reduction").get<uint32_t>()
                };

                generator.add_reduction_percentage(repeats, reduction_percentage);
            }
        }

        generator.validate();
    }
}