#ifndef METRICS_H
#define METRICS_H

#include <operation/pattern.h>

namespace IOMetric {
    struct SyncMetric {
        const char* timestamp;
        OperationPattern::OperationType operation_type;
        int32_t pid;
        int32_t tid;
        uint32_t requested_bytes;
        uint32_t written_bytes;
        int64_t offset;
        int32_t return_code;
        int32_t error_no;
        int64_t duration_us;

        // Dont disponiblize constructor to avoid performance penalty
    };
};

#endif