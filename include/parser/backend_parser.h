#ifndef BACKEND_ENGINE_PARSER_H
#define BACKEND_ENGINE_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <io/backend/posix_engine.h>
#include <parser/metric_parser.h>
#include <parser/logger_parser.h>

namespace Parser {
    using BackendEngineVariant = std::variant<
        BackendEngine::PosixEngine<Logger::Spdlog, std::monostate>,
        BackendEngine::PosixEngine<Logger::Spdlog, Metric::BaseSyncMetric>,
        BackendEngine::PosixEngine<Logger::Spdlog, Metric::StandardSyncMetric>,
        BackendEngine::PosixEngine<Logger::Spdlog, Metric::FullSyncMetric>>;

    inline static const std::unordered_map<
        std::string,
        std::function<BackendEngineVariant(const LoggerVariant& logger, const MetricVariant& metric)>>
    backend_engine_variant_map = {
        {"posix", [](const LoggerVariant& logger_variant,const MetricVariant& metric_variant) {
            return std::visit([&](auto&& actual_logger) -> BackendEngineVariant {
                using ActualLoggerType = std::decay_t<decltype(actual_logger)>;

                return std::visit([&](auto&& actual_metric) -> BackendEngineVariant {
                    using ActualMetricType = std::decay_t<decltype(actual_metric)>;
                    return BackendEngine::PosixEngine<ActualLoggerType, ActualMetricType>(actual_logger);
                }, metric_variant);

            }, logger_variant);
        }}
    };

    BackendEngineVariant getBanckendEngine(const std::string& type, const LoggerVariant& logger, const MetricVariant& metric) {
        auto it  = backend_engine_variant_map.find(type);
        if (it != backend_engine_variant_map.end()) {
            return it->second(logger, metric);
        } else {
            throw std::invalid_argument("Backend engine type '" + type + "' is not recognized");
        }
    }
}

#endif
