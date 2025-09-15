#include <access/pattern.h>
#include <generator/generator.h>
#include <operation/pattern.h>
#include <boost/thread.hpp>
#include <io/logger.h>
#include <chrono>
#include <iostream>


// Generalized function template
template <typename Pattern>
void runPattern(Pattern& pattern, int times) {
    for (int i = 0; i < times; ++i) {
        auto op = pattern.nextOperation();
        std::cout << static_cast<int>(op) << " ";
    }
    std::cout << "\n";
}

int shared_counter = 0;
boost::mutex mtx;

void worker(int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        boost::lock_guard<boost::mutex> lock(mtx);
        shared_counter++;
        std::cout << "Thread " << id << " incremented counter to " << shared_counter << "\n";
    }
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
    IOMetric::SyncMetric syncMetric{
        .timestamp = "2023-10-05T12:00:00Z",
        .operation_type = OperationPattern::OperationType::READ,
        .pid = 1234,
        .tid = 5678,
        .requested_bytes = 4096,
        .written_bytes = 4096,
        .offset = 0,
        .return_code = 0,
        .error_no = 0,
        .duration_us = 150
    };

    logger->info("{}", syncMetric);
    logger->flush();

    const int num_threads = 4;
    const int iterations_per_thread = 5;

    // Create a thread group
    std::vector<boost::thread> threads;

    // Launch threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i, iterations_per_thread);
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Final counter value: " << shared_counter << "\n";
    return 0;
}