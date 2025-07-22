#ifndef OFFSET_STRATEGY_H
#define OFFSET_STRATEGY_H

class OffsetStrategy {

public:
    OffsetStrategy() = default;
    virtual ~OffsetStrategy() = default;
    virtual int next(void) = 0;
};

#endif