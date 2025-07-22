#include <access/distribution/sequential.h>
#include <access/distribution/random.h>
#include <access/distribution/binomial.h>
#include <access/distribution/zipf.h>
#include <access/unreal.h>
#include <iostream>
#include <memory>
#include <fstream>

int main(int argc, char** argv) {
   int iterations = std::stoi(argv[1]);

   std::unique_ptr<Distribution> seq_distribution = std::make_unique<Sequential>(100,10);
   std::unique_ptr<Distribution> rand_distribution = std::make_unique<Random>(100,10);
   std::unique_ptr<Distribution> bin_distribution = std::make_unique<Binomial>(100, 10, 0.5);
   std::unique_ptr<Distribution> zipf_distribution = std::make_unique<Zipf>(100, 10, 0.99);

   std::unique_ptr<Access> seq_access = std::make_unique<UnrealAccess>(std::move(seq_distribution));
   std::unique_ptr<Access> rand_access = std::make_unique<UnrealAccess>(std::move(rand_distribution));
   std::unique_ptr<Access> bin_access = std::make_unique<UnrealAccess>(std::move(bin_distribution));
   std::unique_ptr<Access> zipf_access = std::make_unique<UnrealAccess>(std::move(zipf_distribution));

   std::ofstream sequential_file("sequential.txt");
   std::ofstream random_file("random.txt");
   std::ofstream binomial_file("binomial.txt");
   std::ofstream zipf_file("zipf.txt");

   for (int p = 0; p < iterations; p++) {
      sequential_file << seq_access->next() << std::endl;
      random_file << rand_access->next() << std::endl;
      binomial_file << bin_access->next() << std::endl;
      zipf_file << zipf_access->next() << std::endl;
   }

   sequential_file.close();
   random_file.close();
   binomial_file.close();
   zipf_file.close();

   return 0;
}