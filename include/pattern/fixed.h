#ifndef FIXED_H
#define FIXED_H

#include <vector>
#include <pattern/pattern.h>

class FixedPattern : public Pattern {

private:
    const std::vector<int> pattern;

public:
    FixedPattern() = delete;
    ~FixedPattern() = default;
    AccessType next() override;
};

#endif