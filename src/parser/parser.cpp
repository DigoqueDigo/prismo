#include <parser/parser.h>

namespace Parser {

    std::unique_ptr<Access::Access> getAccess(const json& config) {
        std::string type = config.at("type").get<std::string>();

        if (type == "sequential") {
            return std::make_unique<Access::SequentialAccess>(
                config.get<Access::SequentialAccess>()
            );
        } else if (type == "random") {
            return std::make_unique<Access::RandomAccess>(
                config.get<Access::RandomAccess>()
            );
        } else if (type == "zipfian") {
            return std::make_unique<Access::ZipfianAccess>(
                config.get<Access::ZipfianAccess>()
            );
        } else {
            throw std::invalid_argument("Access type '" + type + "' not recognized");
        }
    }

    std::unique_ptr<Generator::Generator> getGenerator(const json& config) {
        std::string type = config.at("type").get<std::string>();

        if (type == "constant") {
            return std::make_unique<Generator::ConstantGenerator>();
        } else if (type == "random") {
            return std::make_unique<Generator::RandomGenerator>();
        } else {
            throw std::invalid_argument("Generator type '" + type + "' not recognized");
        }
    }

    std::unique_ptr<Operation::Operation> getOperation(const json& config) {
        std::string type = config.at("type").get<std::string>();

        if (type == "constant") {
            return std::make_unique<Operation::ConstantOperation>(
                config.get<Operation::ConstantOperation>()
            );
        } else if (type == "percentage") {
            return std::make_unique<Operation::PercentageOperation>(
                config.get<Operation::PercentageOperation>()
            );
        } else if (type == "sequence") {
            return std::make_unique<Operation::SequenceOperation>(
                config.get<Operation::SequenceOperation>()
            );
        } else {
            throw std::invalid_argument("Operation type '" + type + "' not recognized");
        }
    }

    std::unique_ptr<Operation::MultipleBarrier> getMultipleBarrier(const json& config) {
        return std::make_unique<Operation::MultipleBarrier>(
            config.get<Operation::MultipleBarrier>()
        );
    }

    Metric::MetricType getMetricType(const json& config) {
        std::string type = config.at("metric").get<std::string>();

        if (type == "none") {
            return Metric::MetricType::None;
        } else if (type == "base") {
            return Metric::MetricType::Base;
        } else if (type == "standard") {
             return Metric::MetricType::Standard;
        } else if (type == "full") {
            return Metric::MetricType::Full;
        } else {
            throw std::invalid_argument("Metric type '" + type + "' not recognized");
        }
    }

    std::unique_ptr<Logger::Logger> getLogger(const json& config) {
        const std::string type = config.at("type").get<std::string>();

        if (type == "spdlog") {
            return std::make_unique<Logger::Spdlog>(
                config.get<Logger::SpdlogConfig>()
            );
        } else {
            throw std::invalid_argument("Logger type '" + type + "' not recognized");
        }

    }

    std::unique_ptr<Engine::Engine> getEngine(
        const json& config,
        Metric::MetricType metric_type,
        std::unique_ptr<Logger::Logger> logger
    ) {
        std::string type = config.at("type").get<std::string>();

        if (type == "posix") {
            return std::make_unique<Engine::PosixEngine>(
                metric_type,
                std::move(logger)
            );
        } else if (type == "uring") {
            return std::make_unique<Engine::UringEngine>(
                metric_type,
                std::move(logger),
                config.get<Engine::UringConfig>()
            );
        } else if (type == "aio") {
            return std::make_unique<Engine::AioEngine>(
                metric_type,
                std::move(logger),
                config.get<Engine::AioConfig>()
            );
        } else if (type == "spdk") {
            return std::make_unique<Engine::SpdkEngine>(
                metric_type,
                std::move(logger),
                config.get<Engine::SpdkConfig>()
            );
        } else {
            throw std::invalid_argument("Engine type '" + type + "' not recognized");
        }
    }
}