#include <parser/parser.h>

namespace Parser {

    static const std::unordered_map<
        std::string,
        std::function<EngineVariant(const json&)>>
    engine_variant_map = {
        {"posix", [](const json& specialized) {
            (void) specialized;
            return EngineVariant { std::in_place_type<Engine::PosixEngine> };
        }},
    };

    EngineVariant getEngineVariant(const json& specialized) {
        std::string type = specialized.at("type").template get<std::string>();
        auto it = engine_variant_map.find(type);
        if (it != engine_variant_map.end()) {
            return it->second(specialized);
        } else {
            throw std::invalid_argument("Engine type '" + type + "' not recognized");
        }
    }
}