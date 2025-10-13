#include <parser/parser.h>

namespace Parser {

    static const std::unordered_map<
        std::string,
        std::function<GeneratorVariant()>>
    generator_variant_map = {
        {"constant", []() {
            return GeneratorVariant { std::in_place_type<Generator::ConstantGenerator> };
        }},
        {"random", []() {
            return GeneratorVariant { std::in_place_type<Generator::RandomGenerator> };
        }},
    };

    GeneratorVariant getGeneratorVariant(const json& specialized) {
        std::string type = specialized.at("type").template get<std::string>();
        auto it = generator_variant_map.find(type);
        if (it != generator_variant_map.end()) {
            return it->second();
        } else {
            throw std::invalid_argument("Generator type '" + type + "' not recognized");
        }
    }
}