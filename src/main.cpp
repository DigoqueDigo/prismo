#include <operation/pattern.h>
#include <access/pattern.h>
#include <chrono>
#include <iostream>

int main(int argc, char** argv) {

   int N = 1000; 

   OperationPattern::ReadOperationPattern readOperationPattern;
   OperationPattern::WriteOperationPattern writeOperationPattern;
   OperationPattern::PercentageOperationPattern percentageOperationPattern(30);
   OperationPattern::MixedOperationPattern mixedOperationPattern({
      OperationPattern::OperationType::READ,
      OperationPattern::OperationType::WRITE,
      OperationPattern::OperationType::WRITE,
      OperationPattern::OperationType::READ,
   });

   AccessPattern::SequentialAccessPattern sequentialAccessPattern(10, 2);
   AccessPattern::RandomAccessPattern randomAccessPattern(10, 3);
   AccessPattern::ZipfianAccessPattern zipfianAccessPattern(100, 1, 0.99);

   for (int p = 0; p < N; p++) {
      // std::cout << static_cast<int>(readOperationPattern.nextOperation()) << std::endl;
      // std::cout << static_cast<int>(writeOperationPattern.nextOperation()) << std::endl;
      // std::cout << static_cast<int>(percentageOperationPattern.nextOperation()) << std::endl;
      // std::cout << static_cast<int>(mixedOperationPattern.nextOperation()) << std::endl;

      // std::cout << sequentialAccessPattern.nextOffset() << std::endl;
      // std::cout << randomAccessPattern.nextOffset() << std::endl;
      std::cout << zipfianAccessPattern.nextOffset() << std::endl;
   }

   return 0;
}