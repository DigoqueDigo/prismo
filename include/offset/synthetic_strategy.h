#ifndef SYNTHETIC_STRATEGY_H
#define SYNTHETIC_STRATEGY_H

#include <memory>
#include <offset/offset_strategy.h>
#include <offset/distribution/offset_distribution.h>

class SyntheticStrategy : public OffsetStrategy {

private:
    std::unique_ptr<OffsetDistribution> distribution;

public:
    SyntheticStrategy() = delete;
    explicit SyntheticStrategy(std::unique_ptr<OffsetDistribution> distribution);
    ~SyntheticStrategy() override = default;
    int next() override;
};

#endif