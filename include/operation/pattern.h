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

        explicit ReadOperationPattern()
            : current(OperationType::READ) {}

        OperationType nextOperation() {
            return current;
        }
    };

    struct WriteOperationPattern {
        const OperationType current;

        explicit WriteOperationPattern()
            : current(OperationType::WRITE) {}

        OperationType nextOperation() {
            return current;
        }
    };

    struct MixedOperationPattern {
        size_t index;
        const size_t length;
        const std::vector<OperationType> operations;

        explicit MixedOperationPattern(const std::vector<OperationType>& _operations)
            : index(0), length(_operations.size()), operations(_operations) {}

        OperationType nextOperation() {
            const OperationType operation = operations.at(index);
            index = (index + 1) % length;
            return operation;
        }
    };

    struct PercentageOperationPattern {
        const int read_percentage;
        Distribution::UniformDistribution<int> distribution;

        explicit PercentageOperationPattern(int _read_percentage)
            : read_percentage(_read_percentage), distribution(0, 100) {}

        OperationType nextOperation() {
            return (distribution.nextValue() < read_percentage) ?
                OperationType::READ : OperationType::WRITE;
        }
    };
}

#endif