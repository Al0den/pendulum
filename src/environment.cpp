#include "../include/environment.hpp"
#include "../include/config.hpp"
#include "../include/utils.hpp"
#include "../third-party/physics/include/PhysicsEngine"

#include <sstream>
#include <assert.h>
#include <iostream>

void Environment::createSystem() {
    assert (status == STATUS_UNTRAINED);

    int num_agents = agents.size();

    RigidBodySystem *new_system = new RigidBodySystem();

    new_system->addForceGenerator(new GravityForceGenerator);
    new_system->addForceGenerator(new FrictionForceGenerator(FRICTION_COEFF)); 

    double theta = M_PI;
    for(int i=0; i<num_agents; i++) {
        new_system->addRigidBody(new RigidBody(0, 0, 0, params->base_mass, 0));
        bases.push_back(new_system->getRigidBodiesCount() - 1);
        StaticForceGenerator *staticGenerator = new StaticForceGenerator(new_system->getRigidBodiesCount() - 1, 0, 0);
        BoundForceGenerator *boundGenerator = new BoundForceGenerator(X_BOUND, 1.5, new_system->getRigidBodiesCount() - 1);
        staticGenerator->static_x = false;
        new_system->addForceGenerator(staticGenerator);
        new_system->addForceGenerator(boundGenerator);

        for(int j=0; j<params->pendulum_size; j++) {
            new_system->addRigidBody(new RigidBody(params->pendulum_length * (j + 1) * sin(theta), params->pendulum_length * (j + 1) * cos(theta), 0, params->mass, 0));
            int current = new_system->getRigidBodiesCount() - 1;
            int prev = new_system->getRigidBodiesCount() - 2;
            new_system->addForceGenerator(new SpringForceGenerator(current, prev, params->pendulum_length, SPRING_CONSTANT));
        }
        theta += 0.0001;
    }

    new_system->initialize(new Rk4OdeSolver);
    system = new_system;
}

Environment::Environment(int num_agents, Parameters *params) {
    safety = new std::mutex;

    status = STATUS_UNTRAINED;
    system = nullptr;
    this->params = params;

    for(int i=0; i<num_agents; i++) {
        Agent *agent = new Agent(i * (params->pendulum_size + 1), params->pendulum_size, params);
        this->agents.push_back(agent);
    }

    createSystem();
    generation = 0;
}

Environment::~Environment() {
    if(system != nullptr) {
        delete system;
    }
    for (Agent *agent : agents) {
        delete agent;
    }

    delete safety;
}

void Environment::addToRenderer(rend::RenderEngine *engine) {
    int n = system->getRigidBodiesCount();
    double ball_radius = 0.05;

    for(int i=0; i<n; i++) {
        RigidBody *body = system->getRigidBody(i);
        auto obj = std::make_unique<rend::BallRenderer>(&(body->p_x), &(body->p_y), ball_radius, 255, 255, 255, 0);
        engine->attachObject(std::move(obj)); 
    }
    
    int n_f = system->getForceGeneratorsCount();
    for(int i=0; i<n_f; i++) {
        if(system->getForceGenerator(i)->force_type == FORCE_SPRING) {
            SpringForceGenerator *spring = (SpringForceGenerator*)system->getForceGenerator(i);
            double *p1_x = &(system->getRigidBody(spring->p1_index)->p_x);
            double *p1_y = &(system->getRigidBody(spring->p1_index)->p_y);

            double *p2_x = &(system->getRigidBody(spring->p2_index)->p_x);
            double *p2_y = &(system->getRigidBody(spring->p2_index)->p_y);

            double rest_length = spring->m_restLength;
            auto obj = std::make_unique<rend::SpringRenderer>(p1_x, p1_y, p2_x, p2_y, rest_length, 255, 255, 255, 0);
            engine->attachObject(std::move(obj));
        }
    }
    engine->setZoomFactor(250);
}

std::string Environment::serialize() {
    std::string result = serializeParameters(params) + "\n";
    for(int i=0; i<agents.size(); i++) {
        result += agents[i]->brain->serialize();
        result += "\n";
    }
    return result;
}

void Environment::restore(std::string str) {
    std::vector<std::string> lines;
    std::istringstream f(str);
    std::string line;
    while (std::getline(f, line)) {
        lines.push_back(line);
    }
    agents.clear();
    params = deserializeParameters(lines[0]);
    for(int i=1; i<lines.size(); i++) {
        Agent *agent = new Agent((i - 1) * (params->pendulum_size + 1), params->pendulum_size, params);
        agent->brain->restore(lines[i]);
        agents.push_back(agent);
    }

    createSystem();
}

void Environment::checkBaseData() {
    for(int i=0; i<bases.size(); i++) {
        RigidBody *base = system->getRigidBody(bases[i]);
        if(base->v_x > 10) {
            base->v_x = 10;
        } else if(base->v_x < -10) {
            base->v_x = -10;
        }

        if(base->p_x > X_BOUND - 0.0001) {
            base->v_x = min(base->v_x, 1.0);
        } else if(base->p_x < -X_BOUND + 0.0001) {
            base->v_x = max(base->v_x, -1.0);
        }
    }
}

void Environment::applyPositionChecks() {
    checkBaseData();
}
