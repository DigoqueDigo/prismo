#ifndef CONSTANT_BLOCK_GENERATOR_H
#define CONSTANT_BLOCK_GENERATOR_H

#include <cstring>
#include <generator/block.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace BlockGenerator {
    struct ConstantBlockGeneratorConfig {
        size_t block_size;

        void validate(void) {
            if (block_size == 0)
                throw std::invalid_argument("Invalid block_size for ConstantBlockGeneratorConfig");
        }
    };

    struct ConstantBlockGenerator {
            Block block;

            explicit ConstantBlockGenerator(const ConstantBlockGeneratorConfig& _config) 
                : block(_config.block_size) {}

            const Block& nextBlock(void) {
                std::memset(block.getBuffer(), 0, block.getSize());
                return block;
            }
    };

    void to_json(json& j, const ConstantBlockGeneratorConfig& config) {
        j = json{
            {"type", "constant"},
            {"block_size", config.block_size}
        };
    }

    void from_json(const json& j, ConstantBlockGeneratorConfig& config) {
        j.at("block_size").get_to(config.block_size);
        config.validate();
    }
};

#endif