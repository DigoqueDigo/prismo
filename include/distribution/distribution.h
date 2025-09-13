#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <random>

namespace Distribution {
    template<typename DistributionType> struct UniformDistribution {
        std::mt19937 engine;
        std::uniform_int_distribution<DistributionType> distribution;

        inline UniformDistribution(DistributionType min, DistributionType max)
            : engine(std::random_device{}()), distribution(min, max) {}

        constexpr inline DistributionType nextValue() {
            return distribution(engine);
        }
    };
};

#endif