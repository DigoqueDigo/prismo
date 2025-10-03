#ifndef BACKEND_ENGINE_PARSER_H
#define BACKEND_ENGINE_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <io/metric.h>
#include <io/backend/posix_engine.h>
#include <parser/logger_parser.h>

namespace Parser {
    using BackendEngineVariant = std::variant<
        BackendEngine::PosixEngine<Logger::Spdlog, void>,
        BackendEngine::PosixEngine<Logger::Spdlog, IOMetric::BaseSyncMetric>,
        BackendEngine::PosixEngine<Logger::Spdlog, IOMetric::ThreadSyncMetric>,
        BackendEngine::PosixEngine<Logger::Spdlog, IOMetric::FullSyncMetric>>;

    inline static const std::unordered_map<
        std::string,
        std::function<BackendEngineVariant(const LoggerVariant& logger)>>
    backend_engine_variant_map = {
        {"posix", [](const LoggerVariant& logger_variant) -> BackendEngineVariant {
            return std::visit([](auto&& actual_logger) -> BackendEngineVariant {
                using LoggerType = std::decay_t<decltype(actual_logger)>;
                return BackendEngine::PosixEngine<LoggerType, IOMetric::FullSyncMetric>(actual_logger);
            }, logger_variant);
        }}
    };

    BackendEngineVariant getBanckendEngine(const std::string& type, const LoggerVariant& logger) {
        auto it  = backend_engine_variant_map.find(type);
        if (it != backend_engine_variant_map.end()) {
            return it->second(logger);
        } else {
            throw std::invalid_argument("Access pattern type '" + type + "' is not recognized");
        }
    }
}

#endif
