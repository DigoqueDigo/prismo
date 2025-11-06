#ifndef SPDLOG_CONFIG_LOGGER_H
#define SPDLOG_CONFIG_LOGGER_H

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Logger {

    struct SpdlogConfig {
        std::string name;
        size_t queue_size;
        size_t thread_count;
        bool truncate;
        bool to_stdout;
        std::vector<std::string> files;
    };

    void from_json(const json& j, SpdlogConfig& config);
};

#endif