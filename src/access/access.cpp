#include <access/synthetic.h>

namespace Access {

    Access::Access()
        : block_size(0), limit(0) {}

    Access::Access(size_t _block_size, size_t _limit)
        : block_size(_block_size), limit(_limit) {}

    void Access::validate(void) const {
        if (block_size == 0)
            throw std::invalid_argument("Invalid block_size for AccessConfig");
        if (block_size > limit)
            throw std::invalid_argument("Invalid limit for AccessConfig");
    }

    void from_json(const json& j, Access& base) {
        j.at("block_size").get_to(base.block_size);
        j.at("limit").get_to(base.limit);
    }
}