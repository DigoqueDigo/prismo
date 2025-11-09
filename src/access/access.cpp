#include <access/synthetic.h>

namespace Access {

    void Access::validate(void) const {
        if (block_size == 0)
            throw std::invalid_argument("Invalid block_size for AccessConfig");
        if (block_size > limit)
            throw std::invalid_argument("Invalid limit for AccessConfig");
    }

    void from_json(const json& j, Access& config) {
        j.at("block_size").get_to(config.block_size);
        j.at("limit").get_to(config.limit);
    }
}