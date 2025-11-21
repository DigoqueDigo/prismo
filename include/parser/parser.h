#ifndef PARSER_H
#define PARSER_H

#include <io/metric.h>
#include <access/synthetic.h>
#include <generator/synthetic.h>
#include <operation/synthetic.h>
#include <operation/barrier.h>
#include <engine/engine.h>
#include <engine/aio.h>
#include <engine/posix.h>
#include <engine/uring.h>
// #include <engine/spdk.h>
#include <logger/logger.h>
#include <logger/spdlog.h>

namespace Parser {

    std::unique_ptr<Access::Access> getAccess(const json& config);
    std::unique_ptr<Generator::Generator> getGenerator(const json& config);
    std::unique_ptr<Operation::Operation> getOperation(const json& config);
    std::unique_ptr<Operation::MultipleBarrier> getMultipleBarrier(const json& config);

    std::unique_ptr<Metric::Metric> getMetric(const json& config);
    std::unique_ptr<Logger::Logger> getLogger(const json& config);
    std::unique_ptr<Engine::Engine> getEngine(
        const json& config,
        std::unique_ptr<Metric::Metric> metric,
        std::unique_ptr<Logger::Logger> logger
    );
};

#endif