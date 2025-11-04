#ifndef RANDOM_ACCESS_H
#define RANDOM_ACCESS_H

#include <cstddef>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace Access {

    class RandomAccess {
        private:
            size_t block_size;
            size_t limit;
            Distribution::UniformDistribution<off_t> distribution;

        public:
            RandomAccess() = default;
            off_t nextOffset(void);
            void validate(void) const;
            friend void from_json(const json& j, RandomAccess& config);
    };

    void from_json(const json& j, RandomAccess& config);
}

#endif
