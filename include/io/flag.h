#ifndef POSIX_ENGINE_CONFIG_H
#define POSIX_ENGINE_CONFIG_H

#include <cstdint>
#include <fcntl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Engine {
    inline const std::unordered_map<std::string, int> flag_map = {
        {"O_CREAT", O_CREAT},
        {"O_TRUNC", O_TRUNC},
        {"O_APPEND", O_APPEND},
        {"O_RDONLY", O_RDONLY},
        {"O_WRONLY", O_WRONLY},
        {"O_RDWR", O_RDWR},
        {"O_SYNC", O_SYNC},
        {"O_DSYNC", O_DSYNC},
        {"O_RSYNC", O_RSYNC},
        {"O_DIRECT", O_DIRECT}
    };

    struct Flag {
        int value;
    };

    void from_json(const json& j, Flag& config) {
        for (const auto& value : j.at("flags")) {
            config.value |= flag_map.at(value);
        }
    }
};

#endif
