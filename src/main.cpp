#include <access/distribution/sequential.h>
#include <access/distribution/random.h>
#include <access/distribution/binomial.h>
#include <access/unreal.h>
#include <iostream>
#include <memory>

int main() {
    std::unique_ptr<Distribution> seq_distribution = std::make_unique<Sequential>(100,10);
    std::unique_ptr<Distribution> rand_distribution = std::make_unique<Random>(100,10);
    std::unique_ptr<Distribution> bin_distribution = std::make_unique<Binomial>(100, 10, 0.5);

    std::unique_ptr<Access> seq_access = std::make_unique<UnrealAccess>(std::move(seq_distribution));
    std::unique_ptr<Access> rand_access = std::make_unique<UnrealAccess>(std::move(rand_distribution));
    std::unique_ptr<Access> bin_access = std::make_unique<UnrealAccess>(std::move(bin_distribution));

    for (int p = 0; p < 10000000; p++)
       // std::cout << "sequential: " << seq_access->next() << std::endl;

    for (int p = 0; p < 10000000; p++)
       // std::cout << "random: " << rand_access->next() << std::endl;

    for (int p = 0; p < 10000000; p++)
       // std::cout << "binomial: " << bin_access->next() << std::endl;

    return 0;
}