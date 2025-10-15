#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#include <cstdint>
#include <fcntl.h>
#include <liburing.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Engine {
    
    struct OpenFlags {
        int value;
    };

    struct UringConfig {
        size_t batch;
        size_t block_size;
        uint32_t entries;
        struct io_uring_params params{};

        void validate(void) {
            if (batch > entries) {
                throw std::invalid_argument("Invalid batch for UringConfig");
            }
        }
    };
    
    inline void from_json(const json& j, OpenFlags& config) {
        static const std::unordered_map<std::string, int> flag_map = {
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

        for (const auto& value : j) {
            std::string key = value.template get<std::string>();
            auto it = flag_map.find(key);
            if (it != flag_map.end()) {
                config.value |= it->second;
            } else {
                throw std::invalid_argument("Open flag value '" + key + "' is not recognized");
            }
        }
    }

    inline void from_json(const json& j, UringConfig& config) {
        static const std::unordered_map<std::string, uint32_t> params_flag_map = {
            {"IORING_SETUP_IOPOLL", IORING_SETUP_IOPOLL},
            {"IORING_SETUP_SQPOLL", IORING_SETUP_SQPOLL},
            {"IORING_SETUP_SQ_AFF", IORING_SETUP_SQ_AFF},
            {"IORING_SETUP_CLAMP", IORING_SETUP_CLAMP},
            {"IORING_SETUP_CQSIZE", IORING_SETUP_CQSIZE},
            {"IORING_FEAT_NODROP", IORING_FEAT_NODROP},
            {"IORING_SETUP_SINGLE_ISSUER", IORING_SETUP_SINGLE_ISSUER},
            {"IORING_SETUP_DEFER_TASKRUN", IORING_SETUP_DEFER_TASKRUN}
        };

        j.at("batch").get_to(config.batch);
        j.at("entries").get_to(config.entries);
        j.at("block_size").get_to(config.block_size);

        const json params_j = j.at("params");
        params_j.at("sq_thread_cpu").get_to(config.params.sq_thread_cpu);
        params_j.at("sq_thread_idle").get_to(config.params.sq_thread_idle);

        for (const auto& value : j.at("params").at("flags")) {
            std::string key = value.template get<std::string>();
            auto it = params_flag_map.find(key);
            if (it != params_flag_map.end()) {
                config.params.flags |= it->second;
            } else {
                throw std::invalid_argument("Uring params flag value '" + key + "' is not recognized");
            }
        }

        config.validate();
    };
};

#endif
