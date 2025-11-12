#ifndef METRIC_H
#define METRIC_H

#include <vector>
#include <thread>
#include <variant>
#include <cstdint>
#include <unistd.h>
#include <sys/types.h>
#include <operation/type.h>

namespace Metric {

    enum class MetricType {
        None = 0,
        Base = 1,
        Standard = 2,
        Full = 3,
    };

    struct NoneMetric {};

    struct BaseMetric : NoneMetric {
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

    inline int64_t get_current_timestamp(void) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
    }

    inline void fill_metric(
        MetricType type,
        NoneMetric& metric,
        Operation::OperationType op,
        int64_t start_ts,
        int64_t end_ts,
        ssize_t result,
        size_t size,
        off_t offset
    ) {
        if (type < MetricType::Base)
            return;

        auto& base = static_cast<BaseMetric&>(metric);
        base.operation_type     = op;
        base.start_timestamp    = start_ts;
        base.end_timestamp      = end_ts;

        if (type < MetricType::Standard)
            return;

        auto& standard = static_cast<StandardMetric&>(metric);
        standard.pid = ::getpid();
        standard.tid = static_cast<uint64_t>(
            std::hash<std::thread::id>{}(std::this_thread::get_id())
        );

        if (type < MetricType::Full)
            return;

        auto& full = static_cast<FullMetric&>(metric);
        full.requested_bytes    = size;
        full.offset             = offset;
        full.processed_bytes    = (result > 0) ? static_cast<size_t>(result) : 0;
        full.return_code        = static_cast<int32_t>(result);
        full.error_no           = static_cast<int32_t>(errno);
    }
};

#endif