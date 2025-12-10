#include <access/synthetic.h>

namespace Access {

    void Access::validate(void) const {
        if (block_size == 0)
            throw std::invalid_argument("Invalid block_size for Access");
        if (block_size > limit)
            throw std::invalid_argument("Invalid limit for Access");
    }

    void from_json(const json& j, Access& access_generator) {
        j.at("block_size").get_to(access_generator.block_size);
        j.at("limit").get_to(access_generator.limit);
    }
}