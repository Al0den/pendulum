#pragma once

#include "../include/config.hpp"
#include <random>

class RandomDoubleGenerator {
    private:
        std::mt19937 gen;
        std::uniform_real_distribution<double> dis;

    public:
        RandomDoubleGenerator(double min, double max) 
            : gen(std::random_device{}()), dis(min, max) {}

        double operator()() {
            return dis(gen);
        }
};

int randint(int min, int max);
double max(double a, double b);
double min(double a, double b);

Parameters *defaultParameters();
std::string serializeParameters(Parameters *params);
Parameters *deserializeParameters(std::string serialized);

    
