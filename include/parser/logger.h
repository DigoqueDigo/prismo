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

    LoggerVariant getLogger(const json& specialized) {
        std::string type = specialized.at("type").template get<std::string>();
        auto it = logger_variant_map.find(type);
        if (it != logger_variant_map.end()) {
            return it->second(specialized);
        } else {
            throw std::invalid_argument("Logger type '" + type + "' not recognized");
        }
    }
}

#endif