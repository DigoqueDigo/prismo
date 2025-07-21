#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include <access/access.h>

class Sequential : public Access {

private:
    int current_offset;

public:
    Sequential(int size_limit, int block_size);
    ~Sequential();
    int next(void);
};

#endif