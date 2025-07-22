#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <stdexcept>

class Distribution {

protected:
    const int limit;
    const int block_size;

public:
    Distribution() = delete;
    explicit Distribution(int limit, int block_size);
    virtual ~Distribution() = default;
    virtual int sample() = 0;
};

#endif