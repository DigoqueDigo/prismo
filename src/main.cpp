#include <parser/access.h>
#include <parser/operation.h>
#include <parser/generator.h>
#include <parser/logger.h>
#include <parser/metric.h>
#include <parser/engine.h>

#include <io/logger.h>
#include <operation/type.h>
#include <operation/barrier.h>

#include <iostream>
#include <iomanip>
#include <fstream>

template<
    typename BlockT,
    typename OperationT,
    typename AccessT,
    typename GeneratorT,
    typename EngineT>
void worker(
    const uint32_t iterations,
    const std::string filename,
    Engine::OpenFlags flags,
    Operation::MultipleBarrier barrier,
    BlockT& block,
    OperationT& operation,
    AccessT& access,
    GeneratorT& generator,
    EngineT& engine
) {
    int fd = engine.open(filename.c_str(), flags, 0666);
    
    for (uint32_t i = 0; i < iterations; i++) {
        size_t offset = access.nextOffset();
        
        switch (barrier.apply(operation.nextOperation())) {
            case Operation::OperationType::READ:
                engine.template submit<Operation::OperationType::READ>
                    (fd, block.getBuffer(), block.getSize(), static_cast<off_t>(offset));
                break;
            case Operation::OperationType::WRITE:
                generator.nextBlock(block);
                engine.template submit<Operation::OperationType::WRITE>
                    (fd, block.getBuffer(), block.getSize(), static_cast<off_t>(offset));
                break;
            case Operation::OperationType::FSYNC:
                engine.template submit<Operation::OperationType::FSYNC>
                    (fd, block.getBuffer(), block.getSize(), static_cast<off_t>(offset));
                break;
            case Operation::OperationType::FDATASYNC:
                engine.template submit<Operation::OperationType::FDATASYNC>
                    (fd, block.getBuffer(), block.getSize(), static_cast<off_t>(offset));
                break;
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

    const uint32_t iterations = job_j.at("iterations").template get<uint32_t>();
    const std::string filename = job_j.at("filename").template get<std::string>();

    Engine::OpenFlags flags = engine_j.at("openflags").template get<Engine::OpenFlags>();
    Operation::MultipleBarrier barrier = operation_j.at("barrier").template get<Operation::MultipleBarrier>();

    Generator::BlockConfig block_config = job_j.template get<Generator::BlockConfig>();
    Generator::Block block = Generator::Block(block_config);

    Parser::AccessVariant access = Parser::getAccess(access_j);
    Parser::OperationVariant operation = Parser::getOperation(operation_j);
    Parser::GeneratorVariant generator = Parser::getGenerator(generator_j);

    Parser::MetricVariant metric = Parser::getMetric(job_j);
    Parser::LoggerVariant logger = Parser::getLogger(logging_j);    
    Parser::EngineVariant engine = Parser::getEngine(engine_j, logger, metric);

    std::visit(
        [&iterations, &filename, &flags, &barrier, &block](
            auto& _operation,
            auto& _access,
            auto& _generator,
            auto& _engine) {
            worker(
                iterations,
                filename,
                flags,
                barrier,
                block,
                _operation,
                _access,
                _generator,
                _engine
            );
        },
        operation,
        access,
        generator,
        engine
    );


    // const unsigned int batch_size = 4;
    // const unsigned int queue_depth = 256;
    // const unsigned int ring_flags = IORING_SETUP_SQPOLL | IORING_SETUP_SQ_AFF;

    config_file.close();

    return 0;
}
