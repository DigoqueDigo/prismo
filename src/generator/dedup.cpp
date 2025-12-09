#include <generator/synthetic.h>

namespace Generator {

    DedupGenerator::DedupGenerator()
        : Generator(), distribution(0, 99), percentages(), windows() {}

    uint64_t DedupGenerator::nextBlock(uint8_t* buffer, size_t size) {
        std::memset(buffer, 0, size);
        uint32_t selected_copies = 0;
        uint32_t roll = distribution.nextValue();

        for (const auto& [cumulative, copies] : percentages) {
            if (roll < cumulative) {
                selected_copies = copies;
                break;
            }
        }

        if (selected_copies == 0) {
            std::memcpy(buffer, &block_id, sizeof(block_id));
            return block_id++;
        }

        auto it = windows.find(selected_copies);
        if (it == windows.end()){
            throw std::runtime_error(
                "Can not find window for copies: " + std::to_string(selected_copies));
        }

        std::vector<WindowElement>& window = windows[selected_copies];

        if (window.size() == sliding_window) {
            uint32_t index = distribution.nextValue() % sliding_window;
            WindowElement& element = window[index];

            std::memcpy(buffer, &element.block_id, sizeof(element.block_id));
            element.left_copies--;

            if (element.left_copies == 0) {
                std::swap(window[index], window.back());
                window.pop_back();
            }

            return element.block_id;
        }

        WindowElement element = {
            .block_id = block_id++,
            .left_copies = selected_copies,
        };

        window.push_back(element);
        std::memcpy(buffer, &(element.block_id), sizeof(element.block_id));

        return element.block_id;
    }

    void from_json(const json& j, DedupGenerator& generator) {
        uint32_t cumulative = 0;
        for (const auto& item: j.at("distribution")) {
            uint32_t copies = item.at("copies").get<uint32_t>();
            cumulative += item.at("percentage").get<uint32_t>();
            generator.percentages.emplace_back(cumulative, copies);
            generator.windows.try_emplace(copies);
        }
        if (cumulative != 100) {
            throw std::invalid_argument("Cumulative percentage different of 100");
        }
    }
}