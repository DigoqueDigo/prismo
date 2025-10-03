#ifndef BACKEND_ENGINE_PARSER_H
#define BACKEND_ENGINE_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <io/metric.h>
#include <io/backend/posix_engine.h>

namespace Parser {
    using BackendEngineVariant = std::variant<
        BackendEngine::PosixEngine<void>,
        BackendEngine::PosixEngine<IOMetric::BaseSyncMetric>,
        BackendEngine::PosixEngine<IOMetric::ThreadSyncMetric>,
        BackendEngine::PosixEngine<IOMetric::FullSyncMetric>>;

    inline static const std::unordered_map<
        std::string,
        std::function<BackendEngineVariant(const std::shared_ptr<spdlog::logger>& logger)>>
    backend_engine_variant_map = {
        {"posix", [](const std::shared_ptr<spdlog::logger>& logger) {
            return BackendEngine::PosixEngine<IOMetric::FullSyncMetric>(logger);
        }}
    };

    BackendEngineVariant getBanckendEngine(const std::string& type, const std::shared_ptr<spdlog::logger>& logger) {
        auto it  = backend_engine_variant_map.find(type);
        if (it != backend_engine_variant_map.end()) {
            return it->second(logger);
        } else {
            throw std::invalid_argument("Access pattern type '" + type + "' is not recognized");
        }
    }
}

#endif
