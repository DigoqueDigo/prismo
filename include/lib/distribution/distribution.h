#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <random>
#include <lib/distribution/zipfian.h>

namespace Distribution {
    template<typename DistributionTypeT>
    struct UniformDistribution {
        std::mt19937 engine;
        std::uniform_int_distribution<DistributionTypeT> distribution;

        explicit UniformDistribution()
            : engine(std::random_device{}()), distribution() {}

        explicit UniformDistribution(DistributionTypeT min, DistributionTypeT max)
            : engine(std::random_device{}()), distribution(min, max) {}

        void setParams(DistributionTypeT min, DistributionTypeT max) {
            distribution.param(std::uniform_int_distribution<DistributionTypeT>::param_type(min, max));
        }

        DistributionTypeT nextValue() {
            return distribution(engine);
        }
    };

    template<typename DistributionTypeT>
    struct ZipfianDistribution {
        std::mt19937 engine;
        zipfian_distribution<DistributionTypeT> distribution;

        explicit ZipfianDistribution()
            : engine(std::random_device{}()), distribution() {}

        explicit ZipfianDistribution(DistributionTypeT lower_bound, DistributionTypeT upper_bound, float skew)
            : engine(std::random_device{}()), distribution(lower_bound, upper_bound, skew) {}

        void setParams(DistributionTypeT min, DistributionTypeT max, float skew) {
            distribution.param(zipfian_distribution<DistributionTypeT>::param_type(min, max, skew));
        }

        DistributionTypeT nextValue() {
            return distribution(engine);
        }
    };
};

#endif