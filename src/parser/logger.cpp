#include <parser/parser.h>

namespace Parser {

    static const std::unordered_map<
        std::string,
        std::function<LoggerVariant(const json& specialized)>>
    logger_variant_map = {
        {"spdlog", [](const json& specialized) {
            auto config = specialized.template get<Logger::SpdlogConfig>();
            return LoggerVariant { std::in_place_type<Logger::Spdlog>, config};
        }}
    };

    LoggerVariant getLoggerVariant(const json& specialized) {
        std::string type = specialized.at("type").template get<std::string>();
        auto it = logger_variant_map.find(type);
        if (it != logger_variant_map.end()) {
            return it->second(specialized);
        } else {
            throw std::invalid_argument("Logger type '" + type + "' not recognized");
        }
    }
}