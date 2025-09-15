#ifndef LOGGER_H
#define LOGGER_H

#include <io/metrics.h>
#include <operation/pattern.h>
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

    std::shared_ptr<spdlog::logger> initLogger(const LoggerConfig& config = LoggerConfig{});
};

template<>
struct fmt::formatter<IOMetric::SyncMetric> : fmt::formatter<std::string> {
    auto format(const IOMetric::SyncMetric& syncMetric, fmt::format_context& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(
            ctx.out(),
            "[type={} ts={} pid={} tid={} req={} wrt={} offset={} ret={} errno={} dur={}]",
            static_cast<uint8_t>(syncMetric.operation_type),
            syncMetric.timestamp,
            syncMetric.pid,
            syncMetric.tid,
            syncMetric.requested_bytes,
            syncMetric.written_bytes,
            syncMetric.offset,
            syncMetric.return_code,
            syncMetric.error_no,
            syncMetric.duration_us
        );
    }
};


#endif