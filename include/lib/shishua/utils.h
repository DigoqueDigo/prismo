#ifndef SHISHUA_UTILS_H
#define SHISHUA_UTILS_H

#include <array>
#include <random>
#include <cstdint>

inline std::array<uint64_t, 4> generate_seed(void){
    std::random_device rd;
    std::mt19937_64 rng(rd());

    std::uniform_int_distribution<uint64_t> dist(
        std::numeric_limits<uint64_t>::min(),
        std::numeric_limits<uint64_t>::max()
    );

    std::array<uint64_t, 4> seed{};
    for (auto& s : seed) {
        s = dist(rng);
    }

    return seed;
}

#endif