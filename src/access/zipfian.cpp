#include <access/zipfian.h>
#include <stdexcept>

namespace Access {

    ZipfianAccess::ZipfianAccess()
        : block_size(0), limit(0), skew(0), distribution(0, 99, 0.9f) {}

    size_t ZipfianAccess::nextOffset(void) {
        return distribution.nextValue() * block_size;
    }

    void ZipfianAccess::validate(void) const {
        if (block_size == 0)
            throw std::invalid_argument("Invalid block_size for ZipfianAccessConfig");
        if (block_size > limit)
            throw std::invalid_argument("Invalid limit for ZipfianAccessConfig");
        if (skew <= 0 || skew >= 1)
            throw std::invalid_argument("Invalid skew for ZipfianAccessConfig");
    }

    void from_json(const json& j, ZipfianAccess& config) {
        j.at("block_size").get_to(config.block_size);
        j.at("limit").get_to(config.limit);
        j.at("skew").get_to(config.skew);
        config.validate();
        config.limit = config.limit / config.block_size - 1;
        config.distribution.setParams(0, config.limit, config.skew);
    }
}