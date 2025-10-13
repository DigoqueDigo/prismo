#ifndef PARSER_H
#define PARSER_H

#include <variant>
#include <access/random.h>
#include <access/sequential.h>
#include <access/zipfian.h>
#include <generator/constant.h>
#include <generator/random.h>
#include <operation/constant.h>
#include <operation/percentage.h>
#include <operation/sequence.h>
#include <io/engine/posix.h>
#include <io/logger/spdlog.h>
#include <io/metric.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Parser {

    using AccessVariant = std::variant<
        Access::SequentialAccess,
        Access::RandomAccess,
        Access::ZipfianAccess>;

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
        Engine::PosixEngine>;

    AccessVariant getAccessVariant(const json& specialized);
    GeneratorVariant getGeneratorVariant(const json& specialized);
    OperationVariant getOperationVariant(const json& specialized);
    MetricVariant getMetricVariant(const json& specialized);
    LoggerVariant getLoggerVariant(const json& specialized);
    EngineVariant getEngineVariant(const json& specialized);
};

#endif