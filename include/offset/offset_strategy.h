#ifndef OFFSET_STRATEGY_H
#define OFFSET_STRATEGY_H

class OffsetStrategy {

public:
    OffsetStrategy() = default;
    virtual ~OffsetStrategy() = default;
    virtual size_t next() = 0;
};

#endif