#include <SDL2/SDL.h>

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
        double degrees = theta * 180 / M_PI;

        if(degrees > 120 || degrees < 60) {
            in_a_row = 0;
            return;
        }
    }
    //RigidBody *head = system->getRigidBody(head_id); 
    //RigidBody *end = system->getRigidBody(head_id + num_branches);

    //double theta_dot = sqrt(end->v_x * end->v_x + end->v_y * end->v_y) / params->pendulum_length;
    in_a_row += 1;
    fitness += params->update_dt;
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


void Network::display(int x, int y, int d_width, int d_height, void *renderer_void) {
    SDL_Renderer *renderer = (SDL_Renderer *)renderer_void;
    std::vector<neat::Pair> node_positions;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int current_x = x;
    int x_offset = d_width / layers.size();
    int max_neurons_in_a_layer = 0;

    int neuron_size = 20;

    for (int i=0; i<layers.size(); i++) {
        if(layers[i].size() > max_neurons_in_a_layer) {
            max_neurons_in_a_layer = layers[i].size();
        }
    }
    int y_offset = d_height / max_neurons_in_a_layer;

    for(int i=0; i<layers.size(); i++) {
        int current_y = y + d_height / 2;

        if(layers[i].size() % 2 == 1) {
            current_y = y + d_height/2 - y_offset * (layers[i].size() - 1) / 2;
        } else {
            current_y = y + d_height/2 - y_offset/2 - y_offset * (layers[i].size() - 2) / 2;
        }
        for(int j=0; j<layers[i].size(); j++) {
            int color_grad = (layers[i][j]->out_value + 1)/2 * 255;
            Pair pair = {layers[i][j], current_x, current_y, color_grad};
            node_positions.push_back(pair);
            current_y += y_offset;
        }
        current_x += x_offset;
    }

    for(int i=0; i<connections.size(); i++) {
        Connection *connection = connections[i];
        Node *from = (Node *)connection->from;
        Node *to = (Node *)connection->towards;
        int from_x = 0;
        int from_y = 0;
        int to_x = 0;
        int to_y = 0;
        for(int j=0; j<node_positions.size(); j++) {
            if(node_positions[j].cur_node == from) {
                from_x = node_positions[j].pos_x + (int)(neuron_size / 2);
                from_y = node_positions[j].pos_y + (int)(neuron_size / 2);
            }
            if(node_positions[j].cur_node == to) {
                to_x = node_positions[j].pos_x + (int)(neuron_size / 2);
                to_y = node_positions[j].pos_y + (int)(neuron_size / 2);
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, from_x, from_y, to_x, to_y);
    }
    int current_y;
    current_x = x;
    for(int i=0; i<node_positions.size(); i++) {
        Pair pos = node_positions[i];
        int color_grad = pos.color_grad;
        current_x = pos.pos_x;
        current_y = pos.pos_y;
        SDL_SetRenderDrawColor(renderer, color_grad, 255 - color_grad, 0, 255);
        SDL_Rect neuron = {current_x, current_y, neuron_size, neuron_size};
        SDL_RenderFillRect(renderer, &neuron);
    }

}

