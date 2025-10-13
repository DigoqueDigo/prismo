#ifndef ZIPFIAN_ACCESS_H
#define ZIPFIAN_ACCESS_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace Access {

    struct ZipfianAccess {
        private:
            size_t block_size;
            size_t limit;
            float skew;
            Distribution::ZipfianDistribution<size_t> distribution;

        public:
            ZipfianAccess() = default;
            size_t nextOffset(void);
            void validate(void) const;
            friend void from_json(const json& j, ZipfianAccess& config);
    };

    void from_json(const json& j, ZipfianAccess& config);
}

#endif