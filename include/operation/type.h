#ifndef TYPE_OPERATION_PATTERN_H
#define TYPE_OPERATION_PATTERN_H

namespace OperationPattern {
    enum class OperationType {
        READ,
        WRITE,
        FLUSH,
        FSYNC,
        FDATA_SYNC,
    };
};

#endif