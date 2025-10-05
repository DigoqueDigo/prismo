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
        Metric::BaseSyncMetric,
        Metric::StandardSyncMetric,
        Metric::FullSyncMetric>;

    inline static const std::unordered_map<
        std::string,
        std::function<MetricVariant()>>
    metric_variant_map = {
        {"none", []() {
            return std::monostate{};
        }},
        {"base", []() {
            return Metric::BaseSyncMetric();
        }},
        {"standard", []() {
            return Metric::StandardSyncMetric();
        }},
        {"full", []() {
            return Metric::FullSyncMetric();
        }},
    };

    MetricVariant getMetric(const std::string& type) {
        auto func = metric_variant_map.at(type);
        return func();
    }
}

#endif
