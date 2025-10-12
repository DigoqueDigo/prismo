#ifndef ENGINE_PARSER_H
#define ENGINE_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <io/engine/posix.h>
#include <io/engine/uring.h>
#include <io/flag.h>
#include <parser/metric.h>
#include <parser/logger.h>

namespace Parser {
    using EngineVariant = std::variant<
        Engine::PosixEngine<Logger::Spdlog, std::monostate>,
        Engine::PosixEngine<Logger::Spdlog, Metric::BaseMetric>,
        Engine::PosixEngine<Logger::Spdlog, Metric::StandardMetric>,
        Engine::PosixEngine<Logger::Spdlog, Metric::FullMetric>,
        Engine::UringEngine<Logger::Spdlog, std::monostate>,
        Engine::UringEngine<Logger::Spdlog, Metric::BaseMetric>,
        Engine::UringEngine<Logger::Spdlog, Metric::StandardMetric>,
        Engine::UringEngine<Logger::Spdlog, Metric::FullMetric>>;


    inline static const std::unordered_map<
        std::string,
        std::function<EngineVariant(const json&, const LoggerVariant&, const MetricVariant&)>>
    engine_variant_map = {

        {"posix", [](const json& specialized, const LoggerVariant& logger_variant, const MetricVariant& metric_variant) {
            return std::visit([&](auto&& actual_logger) -> EngineVariant {
                using LoggerT = std::decay_t<decltype(actual_logger)>;
                return std::visit([&](auto&& actual_metric) -> EngineVariant {
                    using MetricT = std::decay_t<decltype(actual_metric)>;
                    return EngineVariant{ std::in_place_type<Engine::PosixEngine<LoggerT, MetricT>>, actual_logger };
                }, metric_variant);
            }, logger_variant);
        }},

        {"uring", [](const json& specialized, const LoggerVariant& logger_variant, const MetricVariant& metric_variant) {
            return std::visit([&](auto&& actual_logger) -> EngineVariant {
                using LoggerT = std::decay_t<decltype(actual_logger)>;
                return std::visit([&](auto&& actual_metric) -> EngineVariant {
                    using MetricT = std::decay_t<decltype(actual_metric)>;
                    Engine::UringConfig config = specialized.template get<Engine::UringConfig>();
                    return EngineVariant{ std::in_place_type<Engine::UringEngine<LoggerT, MetricT>>, config, actual_logger };
                }, metric_variant);
            }, logger_variant);
        }},
    };

    inline EngineVariant getEngine(const json& specialized, const LoggerVariant& logger, const MetricVariant& metric) {
        std::string type = specialized.at("type").template get<std::string>();
        auto it = engine_variant_map.find(type);
        if (it != engine_variant_map.end()) {
            return it->second(specialized, logger, metric);
        } else {
            throw std::invalid_argument("Engine type '" + type + "' not recognized");
        }
    }

}

#endif
