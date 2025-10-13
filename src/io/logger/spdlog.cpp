#include <io/logger/spdlog.h>

namespace Logger {

    Spdlog::Spdlog(const SpdlogConfig& config) {
        std::vector<spdlog::sink_ptr> sinks;
        spdlog::init_thread_pool(config.queue_size, config.thread_count);

        if (config.to_stdout) {
            auto stdout_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
            sinks.push_back(stdout_sink);
        }

        for (auto& file : config.files) {
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file, config.truncate);
            sinks.push_back(file_sink);
        }

        logger = std::make_shared<spdlog::async_logger>(
            config.name,
            sinks.begin(),
            sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );

        spdlog::register_logger(logger);
    }

    void from_json(const json& j, SpdlogConfig& config) {
        j.at("name").get_to(config.name);
        j.at("queue_size").get_to(config.queue_size);
        j.at("thread_count").get_to(config.thread_count);
        j.at("truncate").get_to(config.truncate);
        j.at("stdout").get_to(config.to_stdout);
        j.at("files").get_to(config.files);
    }
};


auto fmt::formatter<Metric::BaseMetric>::format(const Metric::BaseMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(
        ctx.out(),
        "[type={} sts={} ets={}]",
        static_cast<uint8_t>(metric.operation_type),
        metric.start_timestamp,
        metric.end_timestamp
    );
}

auto fmt::formatter<Metric::StandardMetric>::format(const Metric::StandardMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out()) {
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

auto fmt::formatter<Metric::FullMetric>::format(const Metric::FullMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(
        ctx.out(),
        "[type={} sts={} ets={} pid={} tid={} req={} proc={} offset={} ret={} errno={}]",
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