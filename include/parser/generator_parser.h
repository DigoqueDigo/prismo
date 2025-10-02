#ifndef BLOCK_GENERATOR_PARSER_H
#define BLOCK_GENERATOR_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <generator/constant_generator.h>
#include <generator/random_generator.h>

namespace Parser {
    using BlockGeneratorVariant = std::variant<
        BlockGenerator::ConstantBlockGenerator,
        BlockGenerator::RandomBlockGenerator>;

    inline static const std::unordered_map<
        std::string,
        std::function<BlockGeneratorVariant()>>
    block_generator_variant_map = {
        {"constant", []() {
            return BlockGenerator::ConstantBlockGenerator();
        }},
        {"random", []() {
            return BlockGenerator::RandomBlockGenerator();
        }},
    };

    BlockGeneratorVariant getBlockGenerator(const std::string& type) {
        auto it  = block_generator_variant_map.find(type);
        if (it != block_generator_variant_map.end()) {
            return it->second();
        } else {
            throw std::invalid_argument("Block generator type '" + type + "' is not recognized");
        }
    }
}

#endif