#ifndef SPDLOG_LOGGER_H
#define SPDLOG_LOGGER_H

#include <string>
#include <vector>
#include <io/metric.h>
#include <operation/type.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

using json = nlohmann::json;

namespace Logger {
    struct SpdlogConfig {
        std::string name;
        size_t queue_size;
        size_t thread_count;
        bool truncate;
        bool to_stdout;
        std::vector<std::string> files;
    };

    struct Spdlog {
        private:
            std::shared_ptr<spdlog::logger> logger;

        public:
            explicit Spdlog(const SpdlogConfig& config) {
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

                this->logger = std::make_shared<spdlog::async_logger>(
                    config.name,
                    sinks.begin(),
                    sinks.end(),
                    spdlog::thread_pool(),
                    spdlog::async_overflow_policy::block
                );

                spdlog::register_logger(logger);
            }

            template<typename... ArgsT>
            inline void info(ArgsT&&... args) const {
                this->logger->info(std::forward<ArgsT>(args)...);
            }
    };

    void to_json(json& j, const SpdlogConfig& config) {
        j = json{
            {"name", config.name},
            {"queue_size", config.queue_size,
            {"thread_count", config.thread_count},
            {"truncate", config.truncate},
            {"stdout", config.to_stdout},
            {"files", config.files},
        }};
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

template<>
struct fmt::formatter<Metric::BaseSyncMetric> : fmt::formatter<std::string> {
    auto format(const Metric::BaseSyncMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out()) {
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
struct fmt::formatter<Metric::StandardSyncMetric> : fmt::formatter<std::string> {
    auto format(const Metric::StandardSyncMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out()) {
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
struct fmt::formatter<Metric::FullSyncMetric> : fmt::formatter<std::string> {
    auto format(const Metric::FullSyncMetric& metric, fmt::format_context& ctx) const -> decltype(ctx.out()) {
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
};


#endif