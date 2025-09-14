#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <random>
#include <distribution/internals/zipfian.h>

namespace Distribution {
    template<typename DistributionType> struct UniformDistribution {
        std::mt19937 engine;
        std::uniform_int_distribution<DistributionType> distribution;

        explicit UniformDistribution()
            : engine(std::random_device{}()), distribution() {}

        explicit UniformDistribution(DistributionType min, DistributionType max)
            : engine(std::random_device{}()), distribution(min, max) {}

        DistributionType nextValue() {
            return distribution(engine);
        }
    };

    template<typename DistributionType> struct ZipfianDistribution {
        std::mt19937 engine;
        zipfian_distribution<DistributionType> distribution;

        explicit ZipfianDistribution(DistributionType lower_bound, DistributionType upper_bound, float skew_factor)
            : engine(std::random_device{}()), distribution(lower_bound, upper_bound, skew_factor) {}

        DistributionType nextValue() {
            return distribution(engine);
        }
    };
};

#endif