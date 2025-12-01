#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#include <cstdint>
#include <fcntl.h>
#include <libaio.h>
#include <liburing.h>
#include <operation/type.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define MAX_CORES 512

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
        std::string reactor_mask;
        std::string json_config_file;
        uint8_t spdk_threads;
        std::vector<uint64_t> pinned_cores;
    };

    struct MetricData {
        size_t size;
        off_t offset;
        int64_t start_timestamp;
        Operation::OperationType operation_type;
    };

    struct UringUserData {
        int index;
        struct MetricData metric_data;
    };

    struct AioTask {
        void* buffer;
        uint32_t index;
        struct MetricData metric_data;
    };

    void from_json(const json& j, OpenFlags& config);
    void from_json(const json& j, AioConfig& config);
    void from_json(const json& j, UringConfig& config);
    void from_json(const json& j, SpdkConfig& config);

    int count_set_bits(uint64_t mask);
    std::vector<u_int32_t> get_pinned_cores(uint64_t mask);
};

#endif
