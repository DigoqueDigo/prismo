#ifndef PERCENTAGE_PATTERN_H
#define PERCENTAGE_PATTERN_H

#include <random>
#include <stdexcept>
#include <operation/operation_pattern.h>

class PercentagePattern : public OperationPattern {

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    const float read_percentage;

public:
    PercentagePattern() = delete;
    explicit PercentagePattern(float read_percentage);
    ~PercentagePattern() = default;
    AccessType next() override;
};

#endif