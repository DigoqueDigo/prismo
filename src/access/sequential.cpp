#include <access/synthetic.h>

namespace Access {

    SequentialAccess::SequentialAccess()
        : Access(), current_offset(0) {}

    SequentialAccess::SequentialAccess(size_t _block_size, size_t _limit)
        : Access(_block_size, _limit), current_offset(0) {}

    off_t SequentialAccess::nextOffset(void) {
        const off_t offset = current_offset;
        current_offset = static_cast<off_t>((current_offset + block_size) % limit);
        return offset;
    }

    void SequentialAccess::validate(void) const {
        Access::validate();
    }

    void from_json(const json& j, SequentialAccess& config) {
        from_json(j, static_cast<Access&>(config));
        config.validate();
        config.limit = config.block_size * (config.limit / config.block_size);
    }
}
