#include <parser/access_parser.h>
#include <parser/operation_parser.h>
#include <parser/generator_parser.h>
#include <parser/backend_parser.h>

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
    const char* filename,
    _Block& block,
    _OperationPattern& operation_pattern,
    _AccessPattern& access_pattern,
    _BlockGenerator& block_generator,
    _BackendEngine& backend_engine
) {
    int fd = backend_engine._open(filename, O_RDWR | O_CREAT | O_DIRECT, 0666);

    for (int i = 0; i < 100000; i++) {
        block_generator.nextBlock(block);
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
    json run_j = config_j.at("run");




    std::string operation_pattern_key = run_j.at("operation_pattern").template get<std::string>();
    Parser::OperationPatternVariant operation_pattern = Parser::getOperationPattern(
        operation_pattern_key,
        config_j.at("operation_pattern").at(operation_pattern_key)
    );

    std::string access_pattern_key = run_j.at("access_pattern").template get<std::string>();
    Parser::AccessPatternVariant access_pattern = Parser::getAccessPattern(
        access_pattern_key,
        config_j.at("access_pattern").at(access_pattern_key)
    );

    std::string block_generator_key = run_j.at("block_generator").template get<std::string>();
    Parser::BlockGeneratorVariant block_generator = Parser::getBlockGenerator(
        block_generator_key
    );

    BlockGenerator::BlockConfig block_config = config_j.at("block").template get<BlockGenerator::BlockConfig>(); 
    BlockGenerator::Block block(block_config);


    // MELHORAR ESTA MERDA DAQUI PARA BAIXAR
    std::shared_ptr<spdlog::logger> logger = Logger::initLogger();

    std::string backend_engine_key = run_j.at("backend_engine").template get<std::string>();
    Parser::BackendEngineVariant backend_engine = Parser::getBanckendEngine(
        backend_engine_key,
        logger
    );

    std::visit(
        [&block](
            auto& operation_pattern,
            auto& access_pattern,
            auto& block_generator,
            auto& backend_engine) {
            worker("testfile", block, operation_pattern, access_pattern, block_generator, backend_engine);
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


    // OperationPattern::ConstantOperationPattern readOperationPattern;
    // OperationPattern::ConstantOperationPattern writeOperationPattern(OperationPattern::OperationType::WRITE);
    // OperationPattern::PercentageOperationPattern percentageOperationPattern(60);
    // OperationPattern::MixedOperationPattern mixedOperationPattern({
    //     OperationPattern::OperationType::READ,
    //     OperationPattern::OperationType::WRITE,
    //     OperationPattern::OperationType::WRITE,
    //     OperationPattern::OperationType::READ,
    // });

    // json j_read_pattern = readOperationPattern;
    // json j_write_pattern = writeOperationPattern;
    // json j_percentage_pattern = percentageOperationPattern;
    // json j_mixed_pattern = mixedOperationPattern;

    // std::cout << std::setw(4) << j_read_pattern << std::endl;
    // std::cout << std::setw(4) << j_write_pattern << std::endl;
    // std::cout << std::setw(4) << j_percentage_pattern << std::endl;
    // std::cout << std::setw(4) << j_mixed_pattern << std::endl;


    // AccessPattern::SequentialAccessPattern sequentialAccessPattern(size_limit, block_size);
    // AccessPattern::RandomAccessPattern randomAccessPattern(size_limit, block_size);
    // AccessPattern::ZipfianAccessPattern zipfianAccessPattern(size_limit, block_size, zipfian_skew);

    // BlockGenerator::Block block(block_size);
    // BlockGenerator::RandomBlockGenerator randomBlockGenerator(block_size);

    // std::shared_ptr<spdlog::logger> logger = Logger::initLogger();
    // BackendEngine::IOUringConfig ioUringConfig(batch_size, block_size, queue_depth, ring_flags);

    // BackendEngine::PosixEngine<IOMetric::FullSyncMetric> posixEngine(logger);
    // // BackendEngine::IOUringEngine<> ioUringEngine(ioUringConfig, logger);

    // std::cout << std::setw(4) << json::meta() << std::endl;

    // worker(
    //     "testfile_posix",
    //     posixEngine,
    //     writeOperationPattern,
    //     sequentialAccessPattern,
    //     randomBlockGenerator,
    //     block
    // );

    // for (auto& metric : *(posixEngine.metrics)) {
    //     logger->info("{}", metric);
    // }

    // worker(
    //     "testfile_io_uring",
    //     ioUringEngine,
    //     writeOperationPattern,
    //     sequentialAccessPattern,
    //     randomBlockGenerator,
    //     block
    // );

    config_file.close();

    return 0;
}