#ifndef CONFIG_POSIX_BACKEND_ENGINE_H
#define CONFIG_POSIX_BACKEND_ENGINE_H

#include <cstdint>
#include <fcntl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace BackendEngine {
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

    struct PosixConfig {
        int flags;
        uint32_t flush_barrier;
        uint32_t fsync_barrier;
        uint32_t fdata_sync_barrier;
        bool fsync_close; 
    };

    void from_json(const json& j, PosixConfig& config) {
        j.at("barrier").at("flush").get_to(config.flush_barrier);
        j.at("barrier").at("fsync").get_to(config.fsync_barrier);
        j.at("barrier").at("fdata_sync").get_to(config.fdata_sync_barrier);
        j.at("close").at("fsync").get_to(config.fsync_close);
        for (const auto& flag : j.at("flags")) {
            config.flags |= flag_map.at(flag);
        }
    }
};

#endif
