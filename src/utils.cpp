#include "../include/utils.hpp"
#include "../include/config.hpp"

#include <string>
#include <sstream>

int randint(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}

double max(double a, double b) {
    if(a>b) return a;
    return b;
}
double min(double a, double b) {
    if(a<b) return a;
    return b;
}

Parameters *defaultParameters() {
    Parameters *params = new Parameters;
    params->pendulum_size = PENDULUM_SIZE;
    params->pendulum_length = PENDULUM_LENGTH;
    params->mass = PENDULUM_MASS;
    params->base_mass = PENDULUM_MASS * 10;
    params->update_dt = UPDATE_DT;
    params->dt = DT;
    return params;
}

std::string serializeParameters(Parameters *params) {
    return std::to_string(params->pendulum_size) + ";" + std::to_string(params->pendulum_length) + ";" + std::to_string(params->mass) + ";" + std::to_string(params->base_mass) + ";" + std::to_string(params->dt) + ";" + std::to_string(params->update_dt);
}

Parameters *deserializeParameters(std::string serialized) {
    Parameters *params = new Parameters;
    std::vector<std::string> tokens;
    std::istringstream f(serialized);
    std::string token;
    while (std::getline(f, token, ';')) {
        tokens.push_back(token);
    }
    params->pendulum_size = std::stoi(tokens[0]);
    params->pendulum_length = std::stod(tokens[1]);
    params->mass = std::stod(tokens[2]);
    params->base_mass = std::stod(tokens[3]);
    params->dt = std::stod(tokens[4]);
    params->update_dt = std::stod(tokens[5]);
    return params;
};
