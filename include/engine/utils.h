#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#include <cstdint>
#include <fcntl.h>
#include <libaio.h>
#include <liburing.h>
#include <operation/type.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Engine {

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

    struct SpdkConfig {
        std::string bdev_name;
    };

    struct UringUserData {
        size_t size;
        off_t offset;
        uint32_t index;
        int64_t start_timestamp;
        Operation::OperationType operation_type;
    };

    struct AioTask {
        void* buffer;
        size_t size;
        off_t offset;
        uint32_t index;
        int64_t start_timestamp;
        Operation::OperationType operation_type;
    };

    struct SpdkUserData {
        size_t size;
        off_t offset;
        int64_t start_timestamp;
        Operation::OperationType operation_type;
        void* engine;
    };

    void from_json(const json& j, OpenFlags& config);
    void from_json(const json& j, AioConfig& config);
    void from_json(const json& j, UringConfig& config);
    void from_json(const json& j, SpdkConfig& config);
};

#endif
