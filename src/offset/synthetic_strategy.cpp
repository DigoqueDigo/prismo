#include <offset/synthetic_strategy.h>

SyntheticStrategy::SyntheticStrategy(std::unique_ptr<OffsetDistribution> distribution)
    : OffsetStrategy(), distribution(std::move(distribution)) {
        if (!this->distribution) {
            throw std::invalid_argument("Distribution cannot be null");
        }
    }

int SyntheticStrategy::next() {
    return this->distribution->sample();
}