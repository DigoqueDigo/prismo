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
        std::function<AccessVariant(json& specialized, const json& workload)>>
    access__variant_map = {
        {"sequential", [](json& specialized, const json& workload) {
            specialized.merge_patch(workload);
            auto config = specialized.template get<Access::SequentialAccessConfig>();
            return Access::SequentialAccess(config);
        }},
        {"random", [](json& specialized, const json& workload) {
            specialized.merge_patch(workload);
            auto config = specialized.template get<Access::RandomAccessConfig>();
            return Access::RandomAccess(config);
        }},
        {"zipfian", [](json& specialized, const json& workload) {
            specialized.merge_patch(workload);
            auto config = specialized.template get<Access::ZipfianAccessConfig>();
            return Access::ZipfianAccess(config);
        }}
    };

    AccessVariant getAccess(const std::string& type, json& specialized, const json& workload) {
        auto func = access__variant_map.at(type);
        return func(specialized, workload);
    }
}

#endif