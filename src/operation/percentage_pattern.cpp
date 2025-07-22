#include <operation/percentage_pattern.h>

PercentagePattern::PercentagePattern(float read_percentage)
    : OperationPattern(),
        rng(std::random_device{}()), 
        dist(0, 1),
        read_percentage(read_percentage) {
            if (read_percentage < 0 || read_percentage > 1) {
                throw std::invalid_argument("Read percentage must be between 0 and 1");
            }
        }

AccessType PercentagePattern::next() {
    return this->dist(this->rng) < this->read_percentage ? AccessType::READ : AccessType::WRITE;
}