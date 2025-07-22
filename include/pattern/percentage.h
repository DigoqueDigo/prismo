#ifndef PERCENTAGE_H
#define PERCENTAGE_H

#include <random>
#include <stdexcept>
#include <pattern/pattern.h>

class PercentagePattern : public Pattern {

private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;
    std::vector<AccessType> accessOptions;

public:
    PercentagePattern() = delete;
    PercentagePattern(int read_percentage);
    ~PercentagePattern() = default;
    AccessType next() override;
};

#endif