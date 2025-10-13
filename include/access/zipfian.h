#ifndef ZIPFIAN_ACCESS_H
#define ZIPFIAN_ACCESS_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace Access {

    struct ZipfianAccessConfig {
        private:
            size_t block_size;
            size_t limit;
            float skew;

        public:
            constexpr inline size_t getBlockSize(void) const { return block_size; }
            constexpr inline size_t getLimit(void) const { return limit; }
            constexpr inline float getSkew(void) const { return skew; }

            void validate(void) const;
            friend void from_json(const json& j, ZipfianAccessConfig& config);
    };

    struct ZipfianAccess {
        private:
            const ZipfianAccessConfig config;
            Distribution::ZipfianDistribution<size_t> distribution;

        public:
            explicit ZipfianAccess(const ZipfianAccessConfig& _config);
            size_t nextOffset(void);
    };

    void from_json(const json& j, ZipfianAccessConfig& config);
}

#endif