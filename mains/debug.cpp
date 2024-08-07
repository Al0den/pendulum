#include "../include/environment.hpp"
#include "../include/utils.hpp"
#include "../include/config.hpp"
#include "../include/train.hpp"

#include <iostream>

int main() {
    Environment *env = new Environment(1000, defaultParameters());
    RandomDoubleGenerator rdg(0, 1);

    for(int i=0; i<1000000; i++) {
        std::cout << "Batch number: " << i << std::endl;
        env->status = STATUS_RANKED;
        nextBatch(env);
        sanityCheck(env);
    }
}
