#include <io/engine/config.h>

namespace Engine {

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
        {"O_DIRECT", O_DIRECT},
    };

    static const std::unordered_map<std::string, uint32_t> params_flag_map = {
        {"IORING_SETUP_IOPOLL", IORING_SETUP_IOPOLL},
        {"IORING_SETUP_SQPOLL", IORING_SETUP_SQPOLL},
        {"IORING_SETUP_SQ_AFF", IORING_SETUP_SQ_AFF},
        {"IORING_SETUP_CLAMP", IORING_SETUP_CLAMP},
        {"IORING_SETUP_CQSIZE", IORING_SETUP_CQSIZE},
        {"IORING_FEAT_NODROP", IORING_FEAT_NODROP},
        {"IORING_SETUP_SINGLE_ISSUER", IORING_SETUP_SINGLE_ISSUER},
        // {"IORING_SETUP_HYBRID_IOPOLL", IORING_SETUP_HYBRID_IOPOLL},
    };

    void from_json(const json& j, OpenFlags& config) {
        for (const auto& value : j) {
            auto it = flag_map.find(value);
            if (it != flag_map.end()) {
                config.value |= it->second;
            } else {
                throw std::invalid_argument("Open flag value '" + value.template get<std::string>() + "' is not recognized");
            }
        }
    }

    void from_json(const json& j, UringConfig& config) {
        const json params_j = j.at("params");

        j.at("entries").get_to(config.entries);
        j.at("block_size").get_to(config.block_size);

        params_j.at("cq_entries").get_to(config.params.cq_entries);
        params_j.at("sq_thread_cpu").get_to(config.params.sq_thread_cpu);
        params_j.at("sq_thread_idle").get_to(config.params.sq_thread_idle);

        for (const auto& value : j.at("params").at("flags")) {
            auto it = params_flag_map.find(value);
            if (it != params_flag_map.end()) {
                config.params.flags |= it->second;
            } else {
                throw std::invalid_argument("Uring params flag value '" + value.template get<std::string>() + "' is not recognized");
            }
        }
    };
};
