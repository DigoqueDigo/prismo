#include <parser/access_parser.h>
#include <parser/operation_parser.h>
#include <parser/generator_parser.h>
#include <parser/backend_parser.h>
#include <parser/logger_parser.h>

#include <io/logger.h>
#include <operation/type.h>

#include <iostream>
#include <iomanip>
#include <fstream>



template<
    typename _Block,
    typename _OperationPattern,
    typename _AccessPattern,
    typename _BlockGenerator,
    typename _BackendEngine>
void worker(
    const std::string filename,
    _Block& block,
    _OperationPattern& operation_pattern,
    _AccessPattern& access_pattern,
    _BlockGenerator& block_generator,
    _BackendEngine& backend_engine
) {
    int fd = backend_engine._open(filename.c_str(), O_RDWR | O_CREAT | O_DIRECT, 0666);
    block_generator.nextBlock(block);

    for (int i = 0; i < 100000; i++) {
        size_t offset = access_pattern.nextOffset();

        switch (operation_pattern.nextOperation()) {
            case OperationPattern::OperationType::READ:
                backend_engine._read(fd, block.buffer, block.config.size, static_cast<off_t>(offset));
                break;
            case OperationPattern::OperationType::WRITE:
                backend_engine._write(fd, block.buffer, block.config.size, static_cast<off_t>(offset));
                break;
        }
    }

    backend_engine._close(fd);
}




int main(int argc, char** argv) {

    if (argc < 2) {
        throw std::invalid_argument("Invalid number of arguments");
    }
    
    std::ifstream config_file(argv[1]);
    json config_j = json::parse(config_file);
    json workload_j = config_j.at("workload");

    const std::string filename = workload_j.at("filename").template get<std::string>();
    
    BlockGenerator::BlockConfig block_config = config_j.at("workload").template get<BlockGenerator::BlockConfig>(); 
    BlockGenerator::Block block(block_config);

    std::string operation_pattern_key = workload_j.at("operation_pattern").template get<std::string>();
    Parser::OperationPatternVariant operation_pattern = Parser::getOperationPattern(
        operation_pattern_key,
        config_j.at("operation_pattern").at(operation_pattern_key)
    );

    std::string access_pattern_key = workload_j.at("access_pattern").template get<std::string>();
    Parser::AccessPatternVariant access_pattern = Parser::getAccessPattern(
        access_pattern_key,
        config_j.at("access_pattern").at(access_pattern_key),
        workload_j
    );

    std::string block_generator_key = workload_j.at("block_generator").template get<std::string>();
    Parser::BlockGeneratorVariant block_generator = Parser::getBlockGenerator(
        block_generator_key
    );

    std::string logging_key = workload_j.at("logging").template get<std::string>();
    Parser::LoggerVariant logger = Parser::getLoggerVariant(
        logging_key,
        config_j.at("logging").at(logging_key)
    );

    std::string backend_engine_key = workload_j.at("backend_engine").template get<std::string>();
    Parser::BackendEngineVariant backend_engine = Parser::getBanckendEngine(
        backend_engine_key,
        logger
    );

    std::visit(
        [&filename, &block](
            auto& operation_pattern,
            auto& access_pattern,
            auto& block_generator,
            auto& backend_engine) {
            worker(filename, block, operation_pattern, access_pattern, block_generator, backend_engine);
        },
        operation_pattern,
        access_pattern,
        block_generator,
        backend_engine
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