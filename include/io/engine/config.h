#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#include <cstdint>
#include <fcntl.h>
#include <libaio.h>
#include <liburing.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Engine {
    struct OpenMode {
        mode_t value;
    };

    struct OpenFlags {
        int value;
    };

    struct AioConfig {
        size_t block_size;
        uint32_t entries;
    };

    struct UringConfig {
        size_t block_size;
        uint32_t entries;
        io_uring_params params{};
    };

    void from_json(const json& j, OpenFlags& config);
    void from_json(const json& j, AioConfig& config);
    void from_json(const json& j, UringConfig& config);
};

#endif
