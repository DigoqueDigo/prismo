#include <operation/pattern.h>
#include <access/pattern.h>
#include <chrono>
#include <iostream>

int main(int argc, char** argv) {

   int N = 10; 

   OperationPattern::ReadOperationPattern readPattern;
   OperationPattern::WriteOperationPattern writePattern;
   OperationPattern::PercentageOperationPattern percentagePattern(30);
   OperationPattern::MixedOperationPattern mixedPattern({
      OperationPattern::OperationType::READ,
      OperationPattern::OperationType::WRITE,
      OperationPattern::OperationType::WRITE,
      OperationPattern::OperationType::READ,
   });

   AccessPattern::SequentialPattern sequentialPattern(10, 4);
   AccessPattern::RandomPattern randomPattern(10, 3);

   for (int p = 0; p < N; p++) {
      // std::cout << static_cast<int>(readPattern.nextOperation()) << std::endl;
      // std::cout << static_cast<int>(writePattern.nextOperation()) << std::endl;
      // std::cout << static_cast<int>(percentagePattern.nextOperation()) << std::endl;
      // std::cout << static_cast<int>(mixedPattern.nextOperation()) << std::endl;

      // std::cout << sequentialPattern.nextOffset() << std::endl;
      // std::cout << randomPattern.nextOffset() << std::endl;
   }

   return 0;
}