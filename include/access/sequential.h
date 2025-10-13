#ifndef SEQUENTIAL_ACCESS_H
#define SEQUENTIAL_ACCESS_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Access {

    struct SequentialAccessConfig {
        private:
            size_t block_size;
            size_t limit;

        public:
            constexpr inline size_t getBlockSize(void) const { return this->block_size; }
            constexpr inline size_t getLimit(void) const { return this->limit; }

            void validate(void) const;
            friend void from_json(const json& j, SequentialAccessConfig& config);
    };

    struct SequentialAccess {
        private:
            const SequentialAccessConfig config;
            size_t current_offset;

        public:
            explicit SequentialAccess(const SequentialAccessConfig& _config);
            size_t nextOffset(void);
    };

    void from_json(const json& j, SequentialAccessConfig& config);
}

#endif