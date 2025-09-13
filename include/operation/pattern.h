#ifndef OPERATION_PATTERN_H
#define OPERATION_PATTERN_H

#include <vector>
#include <cstdint>
#include <distribution/distribution.h>

namespace OperationPattern {
    enum class OperationType {
        READ,
        WRITE
    };

    struct ReadOperationPattern {
        const OperationType current;

        constexpr inline ReadOperationPattern()
            : current(OperationType::READ) {}

        constexpr inline OperationType nextOperation() {
            return current;
        }
    };

    struct WriteOperationPattern {
        const OperationType current;

        constexpr inline WriteOperationPattern()
            : current(OperationType::WRITE) {}

        constexpr inline OperationType nextOperation() {
            return current;
        }
    };

    struct MixedOperationPattern {
        size_t index;
        const size_t length;
        const std::vector<OperationType> operations;

        inline MixedOperationPattern(const std::vector<OperationType>& operations)
            : index(0), length(operations.size()), operations(operations) {}

        inline OperationType nextOperation() {
            const OperationType operation = operations.at(index);
            index = (index + 1) % length;
            return operation;
        }
    };

    struct PercentageOperationPattern {
        const int read_percentage;
        Distribution::UniformDistribution<int> distribution;

        inline PercentageOperationPattern(int read_percentage)
            : read_percentage(read_percentage), distribution(0, 100) {}

        inline OperationType nextOperation() {
            return (distribution.nextValue() < read_percentage) ?
                OperationType::READ : OperationType::WRITE;
        }
    };
}

#endif