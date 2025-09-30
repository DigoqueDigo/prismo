#ifndef LOGGER_H
#define LOGGER_H

#include <io/metric.h>
#include <operation/type.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace Logger {
    struct LoggerConfig {
        const char* name = "dedisbench";
        const char* filename = "logs/log.txt";
        size_t queue_size = 8192;
        size_t thread_count = 1;
        bool log_to_stdout = true;      
    };

    const std::shared_ptr<spdlog::logger> initLogger(const LoggerConfig& config = LoggerConfig{});
};

template<>
struct fmt::formatter<IOMetric::BaseSyncMetric> : fmt::formatter<std::string> {
    auto format(const IOMetric::BaseSyncMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(
            ctx.out(),
            "[type={} sts={} ets={}]",
            static_cast<uint8_t>(metric.operation_type),
            metric.start_timestamp,
            metric.end_timestamp
        );
    }
};

template<>
struct fmt::formatter<IOMetric::ThreadSyncMetric> : fmt::formatter<std::string> {
    auto format(const IOMetric::ThreadSyncMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(
            ctx.out(),
            "[type={} sts={} ets={} pid={} tid={}]",
            static_cast<uint8_t>(metric.operation_type),
            metric.start_timestamp,
            metric.end_timestamp,
            metric.pid,
            metric.tid
        );
    }
};

template<>
struct fmt::formatter<IOMetric::FullSyncMetric> : fmt::formatter<std::string> {
    auto format(const IOMetric::FullSyncMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(
            ctx.out(),
            "[type={} sts={} ets={} pid={} tid={} req={} wrt={} offset={} ret={} errno={}]",
            static_cast<uint8_t>(metric.operation_type),
            metric.start_timestamp,
            metric.end_timestamp,
            metric.pid,
            metric.tid,
            metric.requested_bytes,
            metric.processed_bytes,
            metric.offset,
            metric.return_code,
            metric.error_no
        );
    }
};


#endif