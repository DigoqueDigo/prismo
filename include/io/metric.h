#ifndef METRIC_H
#define METRIC_H

#include <cstdint>
#include <sys/types.h>
#include <operation/type.h>

namespace Metric {
    struct BaseMetric {
        int64_t start_timestamp;
        int64_t end_timestamp;
        Operation::OperationType operation_type;
    };

    struct StandardMetric : BaseMetric {
        pid_t pid;
        uint64_t tid;
    };

    struct FullMetric : StandardMetric {
        size_t requested_bytes;
        size_t processed_bytes;
        off_t offset;
        int32_t return_code;
        int32_t error_no;
    };
};

#endif