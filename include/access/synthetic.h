#ifndef SYNTHETIC_ACCESS_H
#define SYNTHETIC_ACCESS_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace Access {

    class Access {
        protected:
            size_t block_size;
            size_t limit;

        public:
            Access();
            Access(size_t _block_size, size_t _limit)
                : block_size(_block_size), limit(_limit) {}

            virtual ~Access() = default;
            virtual off_t nextOffset(void) = 0;
            virtual void validate(void) const;
            friend void from_json(const json& j, Access& config);
    };

    class SequentialAccess : public Access {
        private:
            off_t current_offset;

        public:
            SequentialAccess();
            SequentialAccess(size_t _block_size, size_t _limit);

            off_t nextOffset(void) override;
            void validate(void) const;
            friend void from_json(const json& j, SequentialAccess& config);
    };

    class RandomAccess : public Access {
        private:
            Distribution::UniformDistribution<off_t> distribution;

        public:
            RandomAccess();
            RandomAccess(size_t _block_size, size_t _limit);

            off_t nextOffset(void) override;
            void validate(void) const;
            friend void from_json(const json& j, RandomAccess& config);
    };

    class ZipfianAccess : public Access {
        private:
            float skew;
            Distribution::ZipfianDistribution<off_t> distribution;

        public:
            ZipfianAccess();
            ZipfianAccess(size_t _block_size, size_t _limit, float skew);

            off_t nextOffset(void) override;
            void validate(void) const;
            friend void from_json(const json& j, ZipfianAccess& config);
    };

    void from_json(const json& j, Access& config);
    void from_json(const json& j, SequentialAccess& config);
    void from_json(const json& j, RandomAccess& config);
    void from_json(const json& j, ZipfianAccess& config);
};

#endif