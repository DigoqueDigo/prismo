#ifndef TYPE_OPERATION_H
#define TYPE_OPERATION_H

namespace Operation {
    enum class OperationType {
        READ,
        WRITE,
        FSYNC,
        FDATASYNC,
    };
};

#endif