#ifndef PATTERN_H
#define PATTERN_H

#include <vector>
#include <cstdint>
#include <distribution/distribution.h>

namespace Pattern {
    enum class OperationType {
        READ,
        WRITE
    };

    struct ReadOperationPattern {
        OperationType current;

        constexpr inline ReadOperationPattern()
            : current(OperationType::READ) {}

        constexpr inline OperationType nextOperation() {
            return current;
        }
    };

    struct WriteOperationPattern {
        OperationType current;

        constexpr inline WriteOperationPattern()
            : current(OperationType::WRITE) {}

        constexpr inline OperationType nextOperation() {
            return current;
        }
    };

    struct MixedOperationPattern {
        size_t index;
        size_t length;
        OperationType current;
        const std::vector<OperationType> operations;

        inline MixedOperationPattern(const std::vector<OperationType>& operations)
            : index(0), length(operations.size()), current(OperationType::READ), operations(operations) {}

        inline OperationType nextOperation() {
            current = operations.at(index);
            index = (index + 1) % length;
            return current;
        }
    };

    struct PercentageOperationPattern {
        int read_percentage;
        OperationType current;
        Distribution::UniformDistribution<int> distribution;

        inline PercentageOperationPattern(int read_percentage)
            : read_percentage(read_percentage), current(OperationType::READ), distribution(0, 100) {}

        inline OperationType nextOperation() {
            return (distribution.nextValue() < read_percentage) ?
                OperationType::READ : OperationType::WRITE;
        }
    };
}

#endif