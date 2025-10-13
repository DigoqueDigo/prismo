#ifndef ACCESS_PARSER_H
#define ACCESS_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <access/sequential.h>
#include <access/random.h>
#include <access/zipfian.h>

namespace Parser {
    using AccessVariant = std::variant<
        Access::SequentialAccess,
        Access::RandomAccess,
        Access::ZipfianAccess>;

    inline static const std::unordered_map<
        std::string,
        std::function<AccessVariant(json& specialized)>>
    access_variant_map = {
        {"sequential", [](json& specialized) {
            auto access = specialized.template get<Access::SequentialAccess>();
            return AccessVariant { std::in_place_type<Access::SequentialAccess>, std::move(access) };
        }},
        {"random", [](json& specialized) {
            auto access = specialized.template get<Access::RandomAccess>();
            return AccessVariant { std::in_place_type<Access::RandomAccess>, std::move(access) };
        }},
        {"zipfian", [](json& specialized) {
            auto access = specialized.template get<Access::ZipfianAccess>();
            return AccessVariant { std::in_place_type<Access::ZipfianAccess>, std::move(access) };
        }}
    };

    AccessVariant getAccess(json& specialized) {
        std::string type = specialized.at("type").template get<std::string>();
        auto it = access_variant_map.find(type);
        if (it != access_variant_map.end()) {
            return it->second(specialized);
        } else {
            throw std::invalid_argument("Access type '" + type + "' not recognized");
        }
    }
}

#endif