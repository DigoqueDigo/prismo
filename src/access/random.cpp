#include <access/synthetic.h>

namespace Access {

    RandomAccess::RandomAccess()
        : Access(), distribution() {}

    RandomAccess::RandomAccess(size_t _block_size, size_t _limit)
        : Access(_block_size, _limit), distribution(_block_size, _limit) {}

    off_t RandomAccess::nextOffset(void) {
        return static_cast<off_t>(distribution.nextValue() * block_size);
    }

    void RandomAccess::validate(void) const {
        Access::validate();
    }

    void from_json(const json& j, RandomAccess& base) {
        from_json(j, static_cast<Access&>(base));
        base.validate();
        base.limit = base.limit / base.block_size - 1;
        base.distribution.setParams(0, base.limit);
    }
}
