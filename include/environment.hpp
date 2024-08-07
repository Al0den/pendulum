#pragma once

#include "../include/agent.hpp"
#include "../third-party/physics/include/PhysicsEngine"
#include "../third-party/RenderEngine/include/RenderEngine"

#define STATUS_UNTRAINED 0 //Agents that were loaded from a file/just created
#define STATUS_TRAINING 1 //Currently undergoing training
#define STATUS_TRAINED 2 //Finished training, and are ranked by fitness
#define STATUS_RANKED 3
#define STATUS_MUTATED 4 // Have been mutated, those are the agents saved into the file as generation n+1

#include <mutex>
class Environment {
    public:
        Environment(int num_agents, Parameters *params);
        ~Environment();

        std::vector<Agent *> agents;
        std::vector<int> bases;

        RigidBodySystem *system;

        void addToRenderer(rend::RenderEngine *engine);

        std::string serialize();
        void restore(std::string str);
        void checkBaseData();

        void applyPositionChecks();
        void createSystem();

        int status;
        int generation;

        std::mutex *safety;

        Parameters *params;
};
