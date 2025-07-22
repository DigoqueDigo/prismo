#ifndef UNREALACCESS_H
#define UNREALACCESS_H

#include <memory>
#include <access/access.h>
#include <access/distribution/distribution.h>

class UnrealAccess : public Access {

private:
    std::unique_ptr<Distribution> distribution;

public:
    UnrealAccess() = delete;
    UnrealAccess(std::unique_ptr<Distribution> distribution);
    ~UnrealAccess() override = default;
    int next() override;
};

#endif