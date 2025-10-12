#ifndef METRIC_PARSER_H
#define METRIC_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <io/metric.h>

namespace Parser {
    using MetricVariant = std::variant<
        std::monostate,
        Metric::BaseMetric,
        Metric::StandardMetric,
        Metric::FullMetric>;

    inline static const std::unordered_map<
        std::string,
        std::function<MetricVariant()>>
    metric_variant_map = {
        {"none", []() {
            return MetricVariant { std::in_place_type<std::monostate> };
        }},
        {"base", []() {
            return MetricVariant { std::in_place_type<Metric::BaseMetric> };
        }},
        {"standard", []() {
            return MetricVariant { std::in_place_type<Metric::StandardMetric> };
        }},
        {"full", []() {
            return MetricVariant { std::in_place_type<Metric::FullMetric> };
        }},
    };

    MetricVariant getMetric(const json& specialized) {
        std::string type = specialized.at("metric").template get<std::string>();
        auto it = metric_variant_map.find(type);
        if (it != metric_variant_map.end()) {
            return it->second();
        } else {
            throw std::invalid_argument("Metric type '" + type + "' not recognized");
        }
    }
}

#endif
