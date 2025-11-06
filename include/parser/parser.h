#ifndef PARSER_H
#define PARSER_H

#include <io/metric.h>
#include <access/synthetic.h>
#include <generator/synthetic.h>
#include <operation/synthetic.h>
#include <logger/spdlog/spdlog.h>
#include <engine/aio.h>
#include <engine/posix.h>
#include <engine/uring.h>

namespace Parser {

    using MetricVariant = std::variant<
        std::monostate,
        Metric::BaseMetric,
        Metric::StandardMetric,
        Metric::FullMetric
    >;

    using EngineVariant = std::variant<
        Engine::PosixEngine,
        Engine::AioEngine,
        Engine::UringEngine
    >;

    using LoggerVariant = std::variant<
        Logger::Spdlog
    >;

    std::unique_ptr<Access::Access> getAccess(const json& config);
    std::unique_ptr<Generator::Generator> getGenerator(const json& config);
    std::unique_ptr<Operation::Operation> getOperation(const json& config);

    LoggerVariant getLoggerVariant(const json& config);
    EngineVariant getEngineVariant(const json& config);
    MetricVariant getMetricVariant(const json& config);
};

#endif