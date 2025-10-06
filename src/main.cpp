#include <parser/access.h>
#include <parser/operation.h>
#include <parser/generator.h>
#include <parser/logger.h>
#include <parser/metric.h>
#include <parser/engine.h>

#include <io/logger.h>
#include <operation/type.h>

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
    const std::string filename,
    BlockT& block,
    OperationT& operation,
    AccessT& access,
    GeneratorT& generator,
    EngineT& engine
) {
    int fd = engine.open(filename.c_str(), O_CREAT | O_RDWR | O_DIRECT, 0666);
    
    for (int i = 0; i < 100000; i++) {
        generator.nextBlock(block);
        size_t offset = access.nextOffset();

        switch (operation.nextOperation()) {
            case Operation::OperationType::READ:
                engine.template submit<Operation::OperationType::READ>
                    (fd, block.buffer, block.config.size, static_cast<off_t>(offset));
                break;
            case Operation::OperationType::WRITE:
                engine.template submit<Operation::OperationType::WRITE>
                    (fd, block.buffer, block.config.size, static_cast<off_t>(offset));
                break;
            default:
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

    const std::string filename = job_j.at("filename").template get<std::string>();

    Generator::BlockConfig block_config = job_j.template get<Generator::BlockConfig>();
    Generator::Block block = Generator::Block(block_config);

    Parser::AccessVariant access = Parser::getAccess(access_j);
    Parser::OperationVariant operation = Parser::getOperation(operation_j);
    Parser::GeneratorVariant generator = Parser::getGenerator(generator_j);

    Parser::MetricVariant metric = Parser::getMetric(job_j);
    Parser::LoggerVariant logger = Parser::getLogger(logging_j);    
    Parser::EngineVariant engine = Parser::getEngine(engine_j, logger, metric);

    std::visit(
        [&filename, &block](
            auto& _operation,
            auto& _access,
            auto& _generator,
            auto& _engine) {
            worker(
                filename,
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

    // const size_t block_size = 4096;
    // const size_t size_limit = 65536;
    // const float zipfian_skew = 0.99f;
    // const int read_percentage = 30;
    // const unsigned int batch_size = 4;
    // const unsigned int queue_depth = 256;
    // const unsigned int ring_flags = IORING_SETUP_SQPOLL | IORING_SETUP_SQ_AFF;

    config_file.close();

    return 0;
}