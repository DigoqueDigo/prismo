#include <operation/pattern.h>
#include <chrono>
#include <iostream>

int main(int argc, char** argv) {

   // Pattern::ReadOperationPattern pattern;
   // Pattern::WriteOperationPattern pattern;
   // Pattern::PercentageOperationPattern pattern(30);
   Pattern::MixedOperationPattern pattern({
      Pattern::OperationType::READ,
      Pattern::OperationType::WRITE,
      Pattern::OperationType::WRITE,
      Pattern::OperationType::READ,
   });
   
   auto start = std::chrono::high_resolution_clock::now();
   const int N = 10;

   for (int p = 0; p < N; p++) {
      std::cout << static_cast<int>(pattern.nextOperation()) << std::endl;
   }

   auto end = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double> duration = end - start;

   std::cout << "Performed " << N << " operations in " << duration.count() << " seconds.\n";
   std::cout << "Ops per second: " << N / duration.count() << "\n";

   return 0;
}