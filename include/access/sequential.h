#ifndef SEQUENTIAL_ACCESS_H
#define SEQUENTIAL_ACCESS_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Access {

    struct SequentialAccess {
        private:
            size_t block_size;
            size_t limit;
            off_t current_offset;

        public:
            SequentialAccess();
            off_t nextOffset(void);
            void validate(void) const;
            friend void from_json(const json& j, SequentialAccess& config);
    };

    void from_json(const json& j, SequentialAccess& config);
}

#endif