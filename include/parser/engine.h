#ifndef ENGINE_PARSER_H
#define ENGINE_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <io/engine/posix.h>
#include <parser/metric.h>
#include <parser/logger.h>

namespace Parser {
    using EngineVariant = std::variant<
        Engine::PosixEngine<Logger::Spdlog, std::monostate>,
        Engine::PosixEngine<Logger::Spdlog, Metric::BaseSyncMetric>,
        Engine::PosixEngine<Logger::Spdlog, Metric::StandardSyncMetric>,
        Engine::PosixEngine<Logger::Spdlog, Metric::FullSyncMetric>>;

    inline static const std::unordered_map<
        std::string,
        std::function<EngineVariant(const LoggerVariant& logger, const MetricVariant& metric)>>
    engine_variant_map = {
        {"posix", [](const LoggerVariant& logger_variant,const MetricVariant& metric_variant) {
            return std::visit([&](auto&& actual_logger) -> EngineVariant {
                using ActualLoggerType = std::decay_t<decltype(actual_logger)>;

                return std::visit([&](auto&& actual_metric) -> EngineVariant {
                    using ActualMetricType = std::decay_t<decltype(actual_metric)>;
                    return Engine::PosixEngine<ActualLoggerType, ActualMetricType>(actual_logger);
                }, metric_variant);

            }, logger_variant);
        }}
    };

    EngineVariant getEngine(const json& specialized, const LoggerVariant& logger, const MetricVariant& metric) {
        std::string type = specialized.at("type").template get<std::string>();
        auto it = engine_variant_map.find(type);
        if (it != engine_variant_map.end()) {
            return it->second(logger, metric);
        } else {
            throw std::invalid_argument("Engine type '" + type + "' not recognized");
        }
    }
}

#endif
