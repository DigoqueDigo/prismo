#include <operation/fixed_pattern.h>

FixedPattern::FixedPattern(std::unique_ptr<const std::vector<AccessType>> pattern)
    : OperationPattern(),
        current_index(0),
        pattern(std::move(pattern)) {
            if (!this->pattern || !this->pattern->size()) {
                throw std::invalid_argument("Invalid pattern (null or empty)");
            }
        }

AccessType FixedPattern::next() {
    AccessType accessType = this->pattern->at(this->current_index++);
    this->current_index %= this->pattern->size();
    return accessType;
}
