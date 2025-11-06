#ifndef SPDLOG_LOGGER_H
#define SPDLOG_LOGGER_H

#include <io/metric.h>
#include <logger/spdlog/config.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace Logger {

    struct Spdlog {
        private:
            std::shared_ptr<spdlog::logger> logger;

        public:
            Spdlog(const SpdlogConfig& config);

            template<typename... ArgsT>
            inline void info(ArgsT&&... args) const{
                logger->info(std::forward<ArgsT>(args)...);
            }
    };
};

template<>
struct fmt::formatter<Metric::BaseMetric> : fmt::formatter<std::string> {
    auto format(const Metric::BaseMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out());
};

template<>
struct fmt::formatter<Metric::StandardMetric> : fmt::formatter<std::string> {
    auto format(const Metric::StandardMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out());
};

template<>
struct fmt::formatter<Metric::FullMetric> : fmt::formatter<std::string> {
    auto format(const Metric::FullMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out());
};

#endif