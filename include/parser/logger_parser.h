#ifndef LOGGER_PARSER_H
#define LOGGER_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <io/logger.h>

namespace Parser {
    using LoggerVariant = std::variant<
        Logger::Spdlog>;

    inline static const std::unordered_map<
        std::string,
        std::function<LoggerVariant(const json& specialized)>>
    logger_variant_map = {
        {"spdlog", [](const json& specialized) {
            auto config = specialized.template get<Logger::SpdlogConfig>();
            return Logger::Spdlog(config);
        }}
    };

    LoggerVariant getLoggerVariant(const std::string& type, const json& specialized) {
        auto func = logger_variant_map.at(type);
        return func(specialized);
    }
}

#endif