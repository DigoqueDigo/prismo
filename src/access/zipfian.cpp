#include <access/synthetic.h>

namespace Access {

    ZipfianAccess::ZipfianAccess()
        : Access(), skew(0), distribution(0, 99, 0.9f) {}

    ZipfianAccess::ZipfianAccess(size_t _block_size, size_t _limit, float _skew)
        : Access(_block_size, _limit), skew(_skew), distribution(0, _limit, _skew) {}


    off_t ZipfianAccess::nextOffset(void) {
        return static_cast<off_t>(distribution.nextValue() * block_size);
    }

    void ZipfianAccess::validate(void) const {
        Access::validate();
        if (skew <= 0 || skew >= 1)
            throw std::invalid_argument("Invalid skew for ZipfianAccessConfig");
    }

    void from_json(const json& j, ZipfianAccess& base) {
        from_json(j, static_cast<Access&>(base));
        j.at("skew").get_to(base.skew);
        base.validate();
        base.limit = base.limit / base.block_size - 1;
        base.distribution.setParams(0, base.limit, base.skew);
    }
}