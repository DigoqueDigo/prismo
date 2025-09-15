#include <io/logger.h>

namespace Logger {
    std::shared_ptr<spdlog::logger> initLogger(const LoggerConfig& config) {
        std::vector<spdlog::sink_ptr> sinks;
        spdlog::init_thread_pool(config.queue_size, config.thread_count);

        if (config.log_to_stdout) {
            auto stdout_sink = std::make_shared<spdlog::sinks::stdout_sink_mt >();
            sinks.push_back(stdout_sink);
        }

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.filename);
        sinks.push_back(file_sink);

        auto logger = std::make_shared<spdlog::async_logger>(
            config.name,
            sinks.begin(),
            sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );

        spdlog::register_logger(logger);
        return logger;
    }
};