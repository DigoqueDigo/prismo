#include <pattern/percentage.h>

PercentagePattern::PercentagePattern(int read_percentage)
    : Pattern(),
        rng(std::random_device{}()), 
        dist(0, 100), 
        accessOptions(100, AccessType::READ) {
            if (read_percentage < 0 || read_percentage > 100) {
                throw std::invalid_argument("Read percentage must be between 0 and 100");
            } else {
                std::fill(
                    accessOptions.begin() + read_percentage,
                    accessOptions.end(),
                    AccessType::WRITE
                );
            }
        }

AccessType PercentagePattern::next() {
    return this->accessOptions.at(this->dist(rng));
}