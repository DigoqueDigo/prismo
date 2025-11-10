#ifndef METRIC_H
#define METRIC_H

#include <vector>
#include <thread>
#include <variant>
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

    inline int64_t get_current_time(void) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
    }

    template<typename MetricT>
    inline void fill_base_metric(MetricT& metric, Operation::OperationType operation_type, int64_t start_timestamp) {
        if constexpr (std::is_base_of_v<Metric::BaseMetric, MetricT>) {
            metric.operation_type = operation_type;
            metric.start_timestamp = start_timestamp;
            metric.end_timestamp = Metric::get_current_time();
        }
    };

    template<typename MetricT>
    inline void fill_standard_metric(MetricT& metric) {
        if constexpr (std::is_base_of_v<Metric::StandardMetric, MetricT>) {
            metric.pid = ::getpid();
            metric.tid = static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        }
    };

    template<typename MetricT>
    inline void fill_full_metric(MetricT& metric, ssize_t result, size_t size, off_t offset) {
        if constexpr (std::is_base_of_v<Metric::FullMetric, MetricT>) {
            metric.requested_bytes = size;
            metric.processed_bytes = (result > 0) ? static_cast<size_t>(result) : 0;
            metric.offset          = offset;
            metric.return_code     = static_cast<int32_t>(result);
            metric.error_no        = static_cast<int32_t>(errno);
        }
    };

    template<typename MetricT>
    inline void save_on_complete(std::vector<MetricT>& metrics, MetricT& metric) {
        if (std::is_base_of_v<Metric::BaseMetric, MetricT>) {
            metrics.push_back(std::move(metric));
        }
    };

    template<typename MetricT, typename LoggerT>
    inline void log_metric(LoggerT& logger, const MetricT& metric) {
        if constexpr (std::is_base_of_v<Metric::BaseMetric, MetricT>) {
            logger.info(metric);
        }
    };
};

#endif