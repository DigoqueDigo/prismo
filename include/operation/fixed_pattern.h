#ifndef FIXED_PATTERN_H
#define FIXED_PATTERN_H

#include <vector>
#include <memory>
#include <stdexcept>
#include <operation/operation_pattern.h>

class FixedPattern : public OperationPattern {

private:
    int current_index;
    std::unique_ptr<const std::vector<AccessType>> pattern;

public:
    FixedPattern() = delete;
    explicit FixedPattern(std::unique_ptr<const std::vector<AccessType>> pattern);
    ~FixedPattern() = default;
    AccessType next() override;
};

#endif