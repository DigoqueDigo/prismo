#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <random>
#include <distribution/internals/zipfian.h>

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

    template<typename DistributionType> struct ZipfianDistribution {
        std::mt19937 engine;
        zipfian_distribution<DistributionType> distribution;

        inline ZipfianDistribution(DistributionType lower_bound, DistributionType upper_bound, float skew_factor)
            : engine(std::random_device{}()), distribution(lower_bound, upper_bound, skew_factor) {}

        constexpr inline DistributionType nextValue() {
            return distribution(engine);
        }
    };
};

#endif