#include <access/deserialize.h>
#include <operation/deserialize.h>

#include <iostream>
#include <iomanip>
#include <fstream>



// template<typename A, typename B, typename C, typename D, typename E>
// void worker(
//     const char* filename,
//     A& backendEngine,
//     B& operationPattern,
//     C& accessPattern,
//     D& blockGenerator,
//     E& block
//     ) {
//         int fd = backendEngine._open(filename, O_RDWR | O_CREAT | O_DIRECT, 0666);

//         for (int i = 0; i < 20; i++) {
//             blockGenerator.fillBlock(block);
//             unsigned long offset = accessPattern.nextOffset();

//             switch (operationPattern.nextOperation()) {
//                 case OperationPattern::OperationType::READ:
//                     backendEngine._read(fd, block.buffer.data(), block.size, static_cast<off_t>(offset));
//                     break;
//                 case OperationPattern::OperationType::WRITE:
//                     backendEngine._write(fd, block.buffer.data(), block.size, static_cast<off_t>(offset));
//                     break;
//             }
//         }

//         backendEngine._close(fd);
// }


template<
    typename OperationPattern,
    typename AccessPattern>
void foo( 
    OperationPattern operation_pattern, 
    AccessPattern access_pattern
) {
    for (int p = 0; p < 10; p++) {
        std::cout << "Offset: " << tatic_cast<int>(access_pattern.nextOffset()) << std::endl;
        std::cout << "Operation: " << static_cast<int>(operation_pattern.nextOperation()) << std::endl;
    }
}




int main(int argc, char** argv) {
    
    if (argc < 2) {
        throw std::logic_error("Invalid number of arguments");
    }
    
    std::ifstream config_file(argv[1]);
    json data = json::parse(config_file);

    Deserialize::OperationPatternVariant operation_pattern = Deserialize::operation_pattern_variant_map
        .at(data.at("operation_pattern").at("type"))
        (data.at("operation_pattern"));

    Deserialize::AccessPatternVariant access_pattern = Deserialize::access_pattern_variant_map
        .at(data.at("access_pattern").at("type"))
        (data.at("access_pattern"));
    

    std::visit([](auto& a, auto& b){ foo(a, b); }, operation_pattern, access_pattern);

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

    return 0;
}