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

    std::unique_ptr<LoggerVariant> getLoggerVariant(const json& config) {
        const std::string type = config.at("type").get<std::string>();

        if (type == "spdlog") {
            auto logger_config = config.get<Logger::SpdlogConfig>();
            return std::make_unique<LoggerVariant>(
                std::in_place_type<Logger::Spdlog>,
                std::move(logger_config)
            );
        }

        throw std::invalid_argument("Logger type '" + type + "' not recognized");
    }

    std::unique_ptr<EngineVariant> getEngineVariant(const json& config) {
        std::string type = config.at("type").get<std::string>();

        if (type == "posix") {
            return std::make_unique<EngineVariant>(
                std::in_place_type<Engine::PosixEngine>
            );
        } else if (type == "uring") {
            auto engine_config = config.get<Engine::UringConfig>();
            return std::make_unique<EngineVariant>(
                std::in_place_type<Engine::UringEngine>,
                std::move(engine_config)
            );
        } else if (type == "aio") {
            auto engine_config = config.get<Engine::AioConfig>();
            return std::make_unique<EngineVariant>(
                std::in_place_type<Engine::AioEngine>,
                std::move(engine_config)
            );
        } else {
            throw std::invalid_argument("Engine type '" + type + "' not recognized");
        }
    }

    std::unique_ptr<MetricVariant> getMetricVariant(const json& config) {
        std::string type = config.at("metric").get<std::string>();

        if (type == "none") {
            return std::make_unique<MetricVariant>(
                std::in_place_type<std::monostate>
            );
        } else if (type == "base") {
            return std::make_unique<MetricVariant>(
                std::in_place_type<Metric::BaseMetric>
            );
        } else if (type == "standard") {
            return std::make_unique<MetricVariant>(
                std::in_place_type<Metric::StandardMetric>
            );
        } else if (type == "full") {
            return std::make_unique<MetricVariant>(
                std::in_place_type<Metric::FullMetric>
            );
        } else {
            throw std::invalid_argument("Metric type '" + type + "' not recognized");
        }
    }
}