#ifndef METRICS_H
#define METRICS_H

#include <cstdint>
#include <operation/pattern.h>

namespace IOMetric {
    struct SyncMetric {
        uint64_t start_timestamp;
        uint64_t end_timestamp;
        OperationPattern::OperationType operation_type;
        int32_t pid;
        int32_t tid;
        uint32_t requested_bytes;
        uint32_t written_bytes;
        int64_t offset;
        int32_t return_code;
        int32_t error_no;

        // Dont disponiblize constructor to avoid performance penalty
    };

    struct AsyncMetric {
        uint64_t submission_timestamp_us;
        uint64_t completion_timestamp_us;
        OperationPattern::OperationType operation_type;
        int32_t pid;
        int32_t tid;
        uint32_t requested_bytes;
        uint32_t completed_bytes;
        int64_t offset;
        int32_t return_code;
        int32_t error_no;

        // Dont disponiblize constructor to avoid performance penalty
    };
};

#endif