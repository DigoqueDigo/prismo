#include <access/unreal.h>

UnrealAccess::UnrealAccess(std::unique_ptr<Distribution> distribution)
    : Access(), distribution(std::move(distribution)) {
        if (!this->distribution) {
            throw std::invalid_argument("Distribution cannot be null");
        }
    }

int UnrealAccess::next() {
    return this->distribution->sample();
}