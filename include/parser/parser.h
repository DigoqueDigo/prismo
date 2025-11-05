#ifndef PARSER_H
#define PARSER_H

#include <variant>
#include <memory>
#include <access/synthetic.h>


#include <generator/constant.h>
#include <generator/random.h>
#include <operation/constant.h>
#include <operation/percentage.h>
#include <operation/sequence.h>
#include <io/engine/posix.h>
#include <io/engine/uring.h>
#include <io/engine/aio.h>
#include <io/logger/spdlog.h>
#include <io/metric.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Parser {

    using GeneratorVariant = std::variant<
        Generator::ConstantGenerator,
        Generator::RandomGenerator>;

    using OperationVariant = std::variant<
        Operation::ConstantOperation,
        Operation::PercentageOperation,
        Operation::SequenceOperation>;

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

    std::unique_ptr<Access::Access> getAccess(const json& specialized);



    MetricVariant getMetricVariant(const json& specialized);
    LoggerVariant getLoggerVariant(const json& specialized);
    EngineVariant getEngineVariant(const json& specialized);
    GeneratorVariant getGeneratorVariant(const json& specialized);
    OperationVariant getOperationVariant(const json& specialized);
};

#endif