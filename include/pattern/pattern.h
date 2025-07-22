#ifndef PATTERN_H
#define PATTERN_H

enum class AccessType : u_int8_t{
    READ = 0,
    WRITE = 1,
};

class Pattern {

public:
    Pattern() = default;
    virtual ~Pattern() = default;
    virtual AccessType next() = 0;
};

#endif