#include "../include/agent.hpp"
#include "../include/config.hpp"
#include "../include/utils.hpp"

#include <iostream>

Agent::Agent(int head_id, int num_branches, Parameters *params) {
    this->num_branches = num_branches;
    this->brain = new Network(INPUTS_PER_BRANCHES * num_branches + EXTRA_INPUTS, RESPONSE_SIZE, 0);
    this->inputs = nullptr;
    this->outputs = nullptr;

    this->head_id = head_id;
    this->brain->randomizeNetwork();
    this->params = params;
    fitness = 0;
    in_a_row = 0;
}

Agent::~Agent() {
    delete brain;
    if(inputs != nullptr) {
        delete[] inputs;
    }
    if(outputs != nullptr) {
        delete[] outputs;
    }
}


void Agent::getInputs(RigidBodySystem *system) {
    double *inputs_loc = new double[INPUTS_PER_BRANCHES * this->num_branches + EXTRA_INPUTS];

    for(int i=0; i<this->num_branches; i++) {
        int index = this->head_id + i;

        RigidBody *begin = system->getRigidBody(index);
        RigidBody *end = system->getRigidBody(index + 1);

        double theta_dot = sqrt(end->v_x * end->v_x + end->v_y * end->v_y) / params->pendulum_length;

        double theta_dot_sign = (end->v_x * (end->p_y - begin->p_y) - end->v_y * (end->p_x - begin->p_x));
        if(theta_dot_sign > 0) {
            theta_dot_sign = -1;
        } else if (theta_dot_sign < 0) {
            theta_dot_sign = 1;
        } else {
            theta_dot_sign = 0;
        }

        inputs_loc[EXTRA_INPUTS + i * INPUTS_PER_BRANCHES] = (end->p_y - begin->p_x) / params->pendulum_length;
        inputs_loc[EXTRA_INPUTS + i * INPUTS_PER_BRANCHES + 1] = (end->p_x - begin->p_x) / params->pendulum_length;
        inputs_loc[EXTRA_INPUTS + i * INPUTS_PER_BRANCHES + 2] = max(0, min(theta_dot / 10.0, 1.0)) * theta_dot_sign;
    }
    
    RigidBody *head = system->getRigidBody(head_id);
    inputs_loc[0] = max(-1, min(head->v_x / 8, 1));
    inputs_loc[1] = head->p_x / X_BOUND;
    
    if(inputs != nullptr) {
        delete[] inputs;
    }
    inputs = inputs_loc;
}

void Agent::getOutputs() {
    if(outputs != nullptr) {
        delete[] outputs;
    }
    outputs = brain->feed_forward(inputs);
}

void Agent::updateScore(RigidBodySystem *system) {
    for (int i=0; i<num_branches; i++) {
        RigidBody *begin = system->getRigidBody(head_id + i);
        RigidBody *end = system->getRigidBody(head_id + i + 1);

        double theta = atan2(end->p_y - begin->p_y, end->p_x - begin->p_x);

        if(theta < 0) {
            theta += 2 * M_PI;
        }
        double target = M_PI / 2;
        double diff = abs(theta - target) * 180 / M_PI;
        if(diff > 30) {
            in_a_row = 0;
            return;
        }
    }
    //RigidBody *head = system->getRigidBody(head_id); 
    //RigidBody *end = system->getRigidBody(head_id + num_branches);

    //double theta_dot = sqrt(end->v_x * end->v_x + end->v_y * end->v_y) / params->pendulum_length;
    in_a_row += 1;
    fitness += sqrt(in_a_row) * params->update_dt;
}

void Agent::applyMovement(RigidBodySystem *system, double coeff) {
    RigidBody *head = system->getRigidBody(this->head_id);
    if(outputs[0] > 0.5) {
        if(head->v_x < 0) {
            head->v_x *= 0.95;
        }
        head->v_x += 0.16;
    } else if(outputs[0] < -0.5) {
        if(head->v_x > 0) {
            head->v_x *= 0.95;
        }
        head->v_x -= 0.16;
    } else {
        head->v_x *= 0.95;
    }
}
