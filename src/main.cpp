#include <access/pattern.h>
#include <generator/generator.h>
#include <operation/pattern.h>
#include <io/logger.h>
#include <io/backend/engine.h>
#include <boost/thread.hpp>
#include <io/backend/config.h>
#include <chrono>
#include <iostream>
#include <fcntl.h>

template<typename A, typename B, typename C, typename D, typename E>
void worker(
    const char* filename,
    A& backendEngine,
    B& operationPattern,
    C& accessPattern,
    D& blockGenerator,
    E& block
    ) {
        int fd = backendEngine._open(filename, O_RDWR | O_CREAT | O_DIRECT, 0666);

        for (int i = 0; i < 200; i++) {
            blockGenerator.fillBlock(block);
            unsigned long offset = accessPattern.nextOffset();

            switch (operationPattern.nextOperation()) {
                case OperationPattern::OperationType::READ:
                    backendEngine._read(fd, block.buffer.data(), block.size, static_cast<off_t>(offset));
                    break;
                case OperationPattern::OperationType::WRITE:
                    backendEngine._write(fd, block.buffer.data(), block.size, static_cast<off_t>(offset));
                    break;
            }
        }

        backendEngine._close(fd);
}

int main(void) {

    const size_t block_size = 4096;
    const size_t size_limit = 65536;
    const float zipfian_skew = 0.99f;
    const int read_percentage = 30;
    const unsigned int batch_size = 4;
    const unsigned int queue_depth = 256;
    const unsigned int ring_flags = IORING_SETUP_SQPOLL | IORING_SETUP_SQ_AFF;

    OperationPattern::ReadOperationPattern readOperationPattern;
    OperationPattern::WriteOperationPattern writeOperationPattern;
    OperationPattern::PercentageOperationPattern percentageOperationPattern(read_percentage);
    OperationPattern::MixedOperationPattern mixedOperationPattern({
        OperationPattern::OperationType::READ,
        OperationPattern::OperationType::WRITE,
        OperationPattern::OperationType::WRITE,
        OperationPattern::OperationType::READ,
    });

    AccessPattern::SequentialAccessPattern sequentialAccessPattern(size_limit, block_size);
    AccessPattern::RandomAccessPattern randomAccessPattern(size_limit, block_size);
    AccessPattern::ZipfianAccessPattern zipfianAccessPattern(size_limit, block_size, zipfian_skew);

    BlockGenerator::Block block(block_size);
    BlockGenerator::RandomBlockGenerator randomBlockGenerator(block_size);

    std::shared_ptr<spdlog::logger> logger = Logger::initLogger();
    BackendEngineConfig::IOUringConfig ioUringConfig(batch_size, block_size, queue_depth, ring_flags);

    BackendEngine::PosixEngine posixEngine(logger);
    BackendEngine::IOUringEngine ioUringEngine(ioUringConfig, logger);

    worker(
        "testfile",
        posixEngine,
        writeOperationPattern,
        sequentialAccessPattern,
        randomBlockGenerator,
        block
    );

    return 0;
}