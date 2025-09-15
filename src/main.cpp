#include <access/pattern.h>
#include <generator/generator.h>
#include <operation/pattern.h>
#include <io/logger.h>
#include <io/backend/engine.h>
#include <boost/thread.hpp>
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

        if (fd < 0) {
            return;
        }

        blockGenerator.fillBlock(block);

        for (int i = 0; i < 100; i++) {
            off_t offset = accessPattern.nextOffset();
            switch (operationPattern.nextOperation()) {
                case OperationPattern::OperationType::READ:
                    backendEngine._read(fd, block.data.data(), block.length * sizeof(uint64_t), offset);
                    break;
                case OperationPattern::OperationType::WRITE:
                    backendEngine._write(fd, block.data.data(), block.length * sizeof(uint64_t), offset);
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
    BlockGenerator::RandomBlockGenerator randomBlockGenerator;

    std::shared_ptr<spdlog::logger> logger = Logger::initLogger();
    BackendEngine::PosixEngine posixEngine(logger);

    worker(
        "testfile",
        posixEngine,
        readOperationPattern,
        zipfianAccessPattern,
        randomBlockGenerator,
        block
    );

    return 0;
}