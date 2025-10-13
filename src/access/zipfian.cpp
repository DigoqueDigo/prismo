#include <access/zipfian.h>
#include <stdexcept>

namespace Access {

    void ZipfianAccessConfig::validate(void) const {
        if (block_size == 0)
            throw std::invalid_argument("Invalid block_size for ZipfianAccessConfig");
        if (block_size > limit)
            throw std::invalid_argument("Invalid limit for ZipfianAccessConfig");
        if (skew <= 0 || skew >= 1)
            throw std::invalid_argument("Invalid skew for ZipfianAccessConfig");
    }

    ZipfianAccess::ZipfianAccess(const ZipfianAccessConfig& _config)
        : config(_config), distribution(0, _config.getLimit(), _config.getSkew()) {}

    size_t ZipfianAccess::nextOffset(void) {
        return distribution.nextValue() * config.getBlockSize();
    }

    void from_json(const json& j, ZipfianAccessConfig& config) {
        j.at("block_size").get_to(config.block_size);
        j.at("limit").get_to(config.limit);
        j.at("skew").get_to(config.skew);
        config.validate();
        config.limit = config.limit / config.block_size - 1;
    }
}