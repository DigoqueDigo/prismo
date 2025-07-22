#ifndef OPERATION_PATTERN_H
#define OPERATION_PATTERN_H

enum class AccessType : __uint8_t {
    READ = 0,
    WRITE = 1,
};

class OperationPattern {

public:
    OperationPattern() = default;
    virtual ~OperationPattern() = default;
    virtual AccessType next() = 0;
};

#endif