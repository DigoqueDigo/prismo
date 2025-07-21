#include <access/sequential.h>
#include <iostream>
#include <memory>

int main() {
    std::unique_ptr<Access> access = std::make_unique<Sequential>(9, 3); 
    for (int p = 0; p < 10; p++) {
        std::cout << access->next() << std::endl;
    }
    return 0;
}