#include <logger/spdlog/config.h>

namespace Logger {

    void from_json(const json& j, SpdlogConfig& config) {
        config.name         = j.at("name").get<std::string>();
        config.queue_size   = j.at("queue_size").get<size_t>();
        config.thread_count = j.at("thread_count").get<size_t>();
        config.truncate     = j.at("truncate").get<bool>();
        config.to_stdout    = j.at("to_stdout").get<bool>();
        config.files        = j.at("files").get<std::vector<std::string>>();
    };
};
