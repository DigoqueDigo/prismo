#ifndef ACCESS_H
#define ACCESS_H

class Access {

public:
    Access() = default;
    virtual ~Access() = default;
    virtual int next(void) = 0;
};

#endif