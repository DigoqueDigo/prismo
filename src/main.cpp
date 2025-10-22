#include <parser/parser.h>
#include <operation/type.h>
#include <operation/barrier.h>

#include <iostream>
#include <iomanip>
#include <fstream>

template<
    typename OperationT,
    typename AccessT,
    typename GeneratorT,
    typename EngineT,
    typename LoggerT,
    typename MetricT>
void worker(
    const uint64_t iterations,
    const std::string filename,
    Engine::OpenFlags& flags,
    Engine::OpenMode& mode,
    Operation::MultipleBarrier& barrier,
    Generator::Block& block,
    OperationT& operation,
    AccessT& access,
    GeneratorT& generator,
    EngineT& engine,
    LoggerT& logger
) {
    std::vector<MetricT> metrics{};
    metrics.reserve(iterations);

    int fd = engine.open(filename.c_str(), flags, mode);

    for (uint64_t i = 0; i < iterations; ++i) {
        const off_t offset = access.nextOffset();
        const auto op = barrier.apply(operation.nextOperation());

        switch (op) {
            case Operation::OperationType::READ:
                engine.template submit<MetricT, Operation::OperationType::READ>(
                    fd, block.getBuffer(), block.getSize(), offset, metrics
                );
                break;

            case Operation::OperationType::WRITE:
                generator.nextBlock(block);
                engine.template submit<MetricT, Operation::OperationType::WRITE>(
                    fd, block.getBuffer(), block.getSize(), offset, metrics
                );
                break;

            case Operation::OperationType::FSYNC:
                engine.template submit<MetricT, Operation::OperationType::FSYNC>(
                    fd, nullptr, 0, 0, metrics
                );
                break;

            case Operation::OperationType::FDATASYNC:
                engine.template submit<MetricT, Operation::OperationType::FDATASYNC>(
                    fd, nullptr, 0, 0, metrics
                );
                break;

            case Operation::OperationType::NOP:
                engine.template submit<MetricT, Operation::OperationType::NOP>(
                    fd, nullptr, 0, 0, metrics
                );
                break;
        }

        // TODO: melhorar isto
        if constexpr (!std::is_same_v<MetricT, std::monostate>) {
            if (metrics.size() % 1000 == 0) {
                for (const auto& metric : metrics) {
                    logger.info(metric);
                }
                metrics.clear();
            }
        }
    }

    if constexpr (
        std::is_same_v<EngineT, Engine::UringEngine&> ||
        std::is_same_v<EngineT, Engine::AioEngine&>
    ) {
        engine.template reap_left_completions<MetricT>(metrics);
    }

    if constexpr (!std::is_same_v<MetricT, std::monostate>) {
        for (const auto& metric : metrics) {
            logger.info(metric);
        }
    }

    engine.close(fd);
}


int main(int argc, char** argv) {

    if (argc < 2) {
        throw std::invalid_argument("Invalid number of arguments");
    }

    std::ifstream config_file(argv[1]);
    json config_j = json::parse(config_file);

    json job_j = config_j.at("job");
    json operation_j = config_j.at("operation");
    json access_j = config_j.at("access");
    json generator_j = config_j.at("generator");
    json engine_j = config_j.at("engine");
    json logging_j = config_j.at("logging");

    access_j.merge_patch(job_j);
    engine_j.merge_patch(job_j);

    const uint64_t iterations = job_j.at("iterations").template get<uint64_t>();
    const std::string filename = job_j.at("filename").template get<std::string>();

    Engine::OpenMode mode {.value = 0666};
    Engine::OpenFlags flags = engine_j.at("openflags").template get<Engine::OpenFlags>();

    Generator::Block block = job_j.template get<Generator::Block>();
    Operation::MultipleBarrier barrier = operation_j.at("barrier").template get<Operation::MultipleBarrier>();

    Parser::AccessVariant access = Parser::getAccessVariant(access_j);
    Parser::OperationVariant operation = Parser::getOperationVariant(operation_j);
    Parser::GeneratorVariant generator = Parser::getGeneratorVariant(generator_j);

    Parser::MetricVariant metric = Parser::getMetricVariant(job_j);
    Parser::LoggerVariant logger = Parser::getLoggerVariant(logging_j);
    Parser::EngineVariant engine = Parser::getEngineVariant(engine_j);

    std::visit(
        [&iterations, &filename, &flags, &mode, &barrier, &block](
            auto& actual_operation,
            auto& actual_access,
            auto& actual_generator,
            auto& actual_engine,
            auto& actual_logger,
            auto actual_metric) {
            worker<
                decltype(actual_operation),
                decltype(actual_access),
                decltype(actual_generator),
                decltype(actual_engine),
                decltype(actual_logger),
                decltype(actual_metric)
            >(
                iterations,
                filename,
                flags,
                mode,
                barrier,
                block,
                actual_operation,
                actual_access,
                actual_generator,
                actual_engine,
                actual_logger
            );
        },
        operation,
        access,
        generator,
        engine,
        logger,
        metric
    );

    config_file.close();

    return 0;
}
