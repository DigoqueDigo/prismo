#include <offset/distribution/sequential_offset.h>
#include <offset/distribution/random_offset.h>
#include <offset/distribution/binomial_offset.h>
#include <offset/synthetic_strategy.h>
#include <offset/distribution/zipf_offset.h>
#include <generator/const_generator.h>
#include <generator/random_generator.h>
#include <iostream>
#include <memory>
#include <fstream>

int main(int argc, char** argv) {
   // int iterations = std::stoi(argv[1]);

   // std::unique_ptr<OffsetDistribution> seq_distribution = std::make_unique<SequentialOffset>(100, 10);
   // std::unique_ptr<OffsetDistribution> rand_distribution = std::make_unique<RandomOffset>(100, 10);
   // std::unique_ptr<OffsetDistribution> bin_distribution = std::make_unique<BinomialOffset>(100, 10, 0.5);
   // std::unique_ptr<OffsetDistribution> zipf_distribution = std::make_unique<ZipfOffset>(100, 10, 0.99);

   // std::unique_ptr<OffsetStrategy> seq_access = std::make_unique<SyntheticStrategy>(std::move(seq_distribution));
   // std::unique_ptr<OffsetStrategy> rand_access = std::make_unique<SyntheticStrategy>(std::move(rand_distribution));
   // std::unique_ptr<OffsetStrategy> bin_access = std::make_unique<SyntheticStrategy>(std::move(bin_distribution));
   // std::unique_ptr<OffsetStrategy> zipf_access = std::make_unique<SyntheticStrategy>(std::move(zipf_distribution));

   // std::ofstream sequential_file("sequential.txt");
   // std::ofstream random_file("random.txt");
   // std::ofstream binomial_file("binomial.txt");
   // std::ofstream zipf_file("zipf.txt");

   // for (int p = 0; p < iterations; p++) {
   //    sequential_file << seq_access->next() << std::endl;
   //    random_file << rand_access->next() << std::endl;
   //    binomial_file << bin_access->next() << std::endl;
   //    zipf_file << zipf_access->next() << std::endl;
   // }

   // sequential_file.close();
   // random_file.close();
   // binomial_file.close();
   // zipf_file.close();

   std::unique_ptr<BlockGenerator> generator = std::make_unique<RandomGenerator>(4096);
   // std::unique_ptr<BlockGenerator> generator = std::make_unique<ConstGenerator>(4096, std::byte{0x00});
   std::vector<std::vector<std::byte>> blocks;

   for (int p = 0; p < 10; p++) {
      blocks.push_back(std::vector<std::byte>(4096));
   }

   for (int i = 0; i < 10000000; ++i) {
      generator->next(blocks.at(i % 10));
   }

   return 0;
}