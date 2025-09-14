#include <access/pattern.h>
#include <generator/generator.h>
#include <operation/pattern.h>
#include <boost/thread.hpp>
#include <io/logger.h>
#include <chrono>
#include <iostream>


int shared_counter = 0;

// Mutex to protect shared data
boost::mutex mtx;

// Worker function that increments the counter safely
void worker(int id, int increments) {
    for (int i = 0; i < increments; ++i) {
        // Lock the mutex before modifying shared data
        boost::mutex::scoped_lock lock(mtx);
        ++shared_counter;
        std::cout << "Worker " << id << " incremented counter to " << shared_counter << "\n";
        // Mutex automatically unlocks when lock goes out of scope
    }
}


// Generalized function template
template <typename Pattern>
void runPattern(Pattern& pattern, int times) {
    for (int i = 0; i < times; ++i) {
        auto op = pattern.nextOperation();
        std::cout << static_cast<int>(op) << " ";
    }
    std::cout << "\n";
}


int main(void) {

    int N = 1;

    OperationPattern::ReadOperationPattern readOperationPattern;
    OperationPattern::WriteOperationPattern writeOperationPattern;
    OperationPattern::PercentageOperationPattern percentageOperationPattern(30);
    OperationPattern::MixedOperationPattern mixedOperationPattern({
        OperationPattern::OperationType::READ,
        OperationPattern::OperationType::WRITE,
        OperationPattern::OperationType::WRITE,
        OperationPattern::OperationType::READ,
    });

    AccessPattern::SequentialAccessPattern sequentialAccessPattern(10, 3);
    AccessPattern::RandomAccessPattern randomAccessPattern(10, 3);
    AccessPattern::ZipfianAccessPattern zipfianAccessPattern(100, 1, 0.99f);

    BlockGenerator::Block block(80);
    BlockGenerator::RandomBlockGenerator randomBlockGenerator;

    for (int p = 0; p < N; p++) {
        // std::cout << static_cast<int>(readOperationPattern.nextOperation()) << std::endl;
        // std::cout << static_cast<int>(writeOperationPattern.nextOperation()) << std::endl;
        // std::cout << static_cast<int>(percentageOperationPattern.nextOperation()) << std::endl;
        // std::cout << static_cast<int>(mixedOperationPattern.nextOperation()) << std::endl;

        // std::cout << sequentialAccessPattern.nextOffset() << std::endl;
        // std::cout << randomAccessPattern.nextOffset() << std::endl;
        // std::cout << zipfianAccessPattern.nextOffset() << std::endl;

        randomBlockGenerator.fillBlock(block);
        randomBlockGenerator.fillBlock(block);
        randomBlockGenerator.fillBlock(block);
        randomBlockGenerator.fillBlock(block);
        randomBlockGenerator.fillBlock(block);
        randomBlockGenerator.fillBlock(block);

        randomBlockGenerator.fillBlock(block);
        randomBlockGenerator.fillBlock(block);
        randomBlockGenerator.fillBlock(block);
        randomBlockGenerator.fillBlock(block);
        randomBlockGenerator.fillBlock(block);
        randomBlockGenerator.fillBlock(block);

        for (const auto& value : block.data) {
            std::cout << value << "\n";
        }
    }

    runPattern(readOperationPattern, 20);
    runPattern(writeOperationPattern, 20);
    runPattern(mixedOperationPattern, 20);
    runPattern(percentageOperationPattern, 20);

    boost::thread t1(worker, 1, 5);
    boost::thread t2(worker, 2, 5);

    t1.join();
    t2.join();

    auto logger = Logger::initLogger();
    IOMetric::SyncMetric syncMetric;

    logger->info("{}", syncMetric);
    logger->flush();
    return 0;
}