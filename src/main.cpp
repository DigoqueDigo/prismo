#include <access/pattern.h>
#include <generator/generator.h>
#include <operation/pattern.h>
#include <chrono>
#include <iostream>

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
    AccessPattern::ZipfianAccessPattern zipfianAccessPattern(100, 1, 0.99);

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

        for (const auto& value : block.data) {
            std::cout << value << "\n";
        }
    }

    return 0;
}