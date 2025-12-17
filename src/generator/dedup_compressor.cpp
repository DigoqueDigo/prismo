#include <generator/dedup_compressor.h>

namespace Generator {

    DedupCompressorGenerator::DedupCompressorGenerator(const DedupCompressorGeneratorConfig& _config)
        : Generator(), pool(_config.buffer_size), config(_config), distribution(0, 99) {
            refill_buffer = std::make_unique<uint8_t[]>(config.buffer_size);
            random_generator.next_block(refill_buffer.get(), config.buffer_size);
        }

    BlockMetadata DedupCompressorGenerator::next_block(uint8_t* buffer, size_t size) {
        uint32_t dedup_roll = distribution.nextValue();
        uint32_t reduction_roll = distribution.nextValue();

        uint32_t selected_repeats = config.select_repeats(dedup_roll);
        uint32_t selected_reduction = config.select_reduction(reduction_roll, selected_repeats);

        if (selected_repeats == 0) {
            DedupElement element = create_dedup_element(selected_repeats, selected_reduction, size);
            return BlockMetadata {
                .block_id = element.block_id,
                .compression = element.reduction,
            };
        }

        if (dedup_windows[selected_repeats].size() == DEDUP_WINDOW_SIZE) {
            DedupElement element = reuse_dedup_element(selected_repeats, buffer, size);
            return BlockMetadata {
                .block_id = element.block_id,
                .compression = element.reduction,
            };
        }

        DedupElement element = create_dedup_element(selected_repeats, selected_reduction, size);
        dedup_windows[selected_repeats].push_back(element);
        std::memcpy(buffer, element.buffer, size);

        return BlockMetadata {
            .block_id = element.block_id,
            .compression = selected_reduction
        };
    }

    DedupElement DedupCompressorGenerator::reuse_dedup_element(
        uint32_t repeats,
        uint8_t* buffer,
        size_t size
    ) {
        auto& dedup_window = dedup_windows[repeats];
        uint32_t index = distribution.nextValue() % dedup_window.size();
        DedupElement& element = dedup_windows[repeats][index];

        element.left_repeats--;
        std::memcpy(buffer, element.buffer, size);

        if (element.left_repeats == 0) {
            std::swap(dedup_window[index], dedup_window.back());
            dedup_window.pop_back();
            pool.free(element.buffer);
        }

        return element;
    }

    DedupElement DedupCompressorGenerator::create_dedup_element(
        uint32_t repeats,
        uint32_t reduction,
        size_t size
    ) {
        DedupElement element = {
            .block_id = block_id++,
            .left_repeats = repeats,
            .reduction = reduction,
            .buffer = static_cast<uint8_t*>(pool.malloc()),
        };

        uint32_t bytes_reduction = size * reduction / 100.0;

        std::memset(element.buffer, 0, bytes_reduction);
        std::memcpy(element.buffer, &element.block_id, sizeof(element.block_id));

        if (config.refill_buffers) {
            random_generator.next_block(
                element.buffer + bytes_reduction,
                size - bytes_reduction
            );
        } else {
            std::memcpy(
                element.buffer + bytes_reduction,
                refill_buffer.get() + bytes_reduction,
                size - bytes_reduction
            );
        }

        return element;
    }

    void DedupCompressorGeneratorConfig::add_dedup_percentage(
        PercentageElement<uint32_t, uint32_t> dedup_percentage
    ) {
        dedup_percentages.push_back(dedup_percentage);
    }

    void DedupCompressorGeneratorConfig::add_reduction_percentage(
        uint32_t repeats,
        PercentageElement<uint32_t, uint32_t> reduction_percentage
    ) {
        reduction_percentages[repeats].push_back(reduction_percentage);
    }

    uint32_t DedupCompressorGeneratorConfig::select_repeats(uint32_t roll) {
        return select_from_percentage_vector(roll, dedup_percentages);
    };

    uint32_t DedupCompressorGeneratorConfig::select_reduction(uint32_t roll, uint32_t repeats) {
        return select_from_percentage_vector(roll, reduction_percentages[repeats]);
    }

    void DedupCompressorGeneratorConfig::validate() const {
        validate_percentage_vector(dedup_percentages, "dedup");
        for (const auto& [key, vec] : reduction_percentages) {
            validate_percentage_vector(vec, "reduction for repeats " + std::to_string(key));

            bool has_invalid_reduction = std::any_of(vec.begin(), vec.end(),
                [](const auto& elem){ return elem.value > 100; });

            if (has_invalid_reduction) {
                throw std::invalid_argument("validate: reduction value must be less than 100");
            }
        }
    }

    void from_json(const json& j, DedupCompressorGeneratorConfig& config) {
        uint32_t dedup_cumulative = 0;
        uint32_t reduction_cumulative = 0;

        j.at("block_size").get_to(config.buffer_size);
        j.at("refill_buffers").get_to(config.refill_buffers);

        for (const auto& dedup_item : j.at("distribution")) {
            uint32_t repeats = dedup_item.at("repeats").get<uint32_t>();
            dedup_cumulative += dedup_item.at("percentage").get<uint32_t>();

            config.add_dedup_percentage(
                PercentageElement<uint32_t, uint32_t> {
                    .cumulative_percentage = dedup_cumulative,
                    .value = repeats,
            });

            reduction_cumulative = 0;

            for (const auto& reduction_item : dedup_item.at("compression")) {
                reduction_cumulative += reduction_item.at("percentage").get<uint32_t>();
                uint32_t reduction_value = reduction_item.at("reduction").get<uint32_t>();

                config.add_reduction_percentage(repeats,
                    PercentageElement<uint32_t, uint32_t> {
                        .cumulative_percentage = reduction_cumulative,
                        .value = reduction_value,
                });
            }
        }

        config.validate();
    }
}