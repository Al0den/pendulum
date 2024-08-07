#pragma once

#include "../third-party/neat/neat.hpp"
#include "../include/config.hpp"

#include "../third-party/physics/include/PhysicsEngine"

using namespace neat;

class Agent {
    public:
        Agent(int head_id, int num_branches, Parameters *params);
        ~Agent();

        Network *brain;

        void getInputs(RigidBodySystem *system);
        void getOutputs();
        void updateScore(RigidBodySystem *system);
        void applyMovement(RigidBodySystem *system, double coeff);

        int num_branches;

        double *inputs;
        double *outputs;

        double fitness;

        int head_id;
        int in_a_row;

        Parameters *params;
};
