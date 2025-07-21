#ifndef ACCESS_H
#define ACCESS_H

class Access {

protected:
    const int size_limit;
    const int block_size;

public:
    Access(int size_limit, int block_size);
    virtual ~Access();
    virtual int next(void) = 0;
};

#endif