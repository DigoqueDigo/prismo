#include <parser/parser.h>

namespace Parser {

    static const std::unordered_map<
        std::string,
        std::function<AccessVariant(const json& specialized)>>
    access_variant_map = {
        {"sequential", [](const json& specialized) {
            auto access = specialized.template get<Access::SequentialAccess>();
            return AccessVariant { std::in_place_type<Access::SequentialAccess>, std::move(access) };
        }},
        {"random", [](const json& specialized) {
            auto access = specialized.template get<Access::RandomAccess>();
            return AccessVariant { std::in_place_type<Access::RandomAccess>, std::move(access) };
        }},
        {"zipfian", [](const json& specialized) {
            auto access = specialized.template get<Access::ZipfianAccess>();
            return AccessVariant { std::in_place_type<Access::ZipfianAccess>, std::move(access) };
        }}
    };

    AccessVariant getAccessVariant(const json& specialized) {
        std::string type = specialized.at("type").template get<std::string>();
        auto it = access_variant_map.find(type);
        if (it != access_variant_map.end()) {
            return it->second(specialized);
        } else {
            throw std::invalid_argument("Access type '" + type + "' not recognized");
        }
    }
}