#ifndef PARSER_H
#define PARSER_H

#include <variant>
#include <memory>
#include <access/synthetic.h>
#include <generator/synthetic.h>
#include <operation/synthetic.h>


#include <io/engine/posix.h>
#include <io/engine/uring.h>
#include <io/engine/aio.h>
#include <io/logger/spdlog.h>
#include <io/metric.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Parser {


    using MetricVariant = std::variant<
        std::monostate,
        Metric::BaseMetric,
        Metric::StandardMetric,
        Metric::FullMetric>;

    using LoggerVariant = std::variant<
        Logger::Spdlog>;

    using EngineVariant = std::variant<
        Engine::PosixEngine,
        Engine::UringEngine,
        Engine::AioEngine>;

    std::unique_ptr<Access::Access> getAccess(const json& config);
    std::unique_ptr<Generator::Generator> getGenerator(const json& config);
    std::unique_ptr<Operation::Operation> getOperation(const json& config);



    MetricVariant getMetricVariant(const json& config);
    LoggerVariant getLoggerVariant(const json& config);
    EngineVariant getEngineVariant(const json& config);
};

#endif