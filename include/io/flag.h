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

    struct Flags {
        int value;
    };

    void from_json(const json& j, Flags& config) {
        for (const auto& value : j) {
            std::string key = value.template get<std::string>();
            auto it = flag_map.find(key);
            if (it != flag_map.end()) {
                config.value |= it->second;
            } else {
                throw std::invalid_argument("Flag value '" + key + "' is not recognized");
            }
        }
    }
};

#endif
