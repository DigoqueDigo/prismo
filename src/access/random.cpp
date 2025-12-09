#include <access/synthetic.h>

namespace Access {

    RandomAccess::RandomAccess()
        : Access(), distribution() {}

    RandomAccess::RandomAccess(size_t _block_size, size_t _limit)
        : Access(_block_size, _limit), distribution(_block_size, _limit) {}

    uint64_t RandomAccess::nextOffset(void) {
        return static_cast<uint64_t>(distribution.nextValue() * block_size);
    }

    void RandomAccess::validate(void) const {
        Access::validate();
    }

    void from_json(const json& j, RandomAccess& config) {
        from_json(j, static_cast<Access&>(config));
        config.validate();
        config.limit = config.limit / config.block_size - 1;
        config.distribution.setParams(0, config.limit);
    }
}
