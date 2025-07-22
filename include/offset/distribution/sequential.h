#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include <access/distribution/distribution.h>

class Sequential : public Distribution {

private:
    int current_offset;

public:
    Sequential() = delete;
    explicit Sequential(int limit, int block_size);
    ~Sequential() override = default;
    int sample() override;
};

#endif