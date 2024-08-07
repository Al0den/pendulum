#include "../include/environment.hpp"
#include "../include/train.hpp"
#include "../include/utils.hpp"

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <assert.h>
#include <string>
#include <sstream>
#include <filesystem>
#include <algorithm>

TrainingInfo *info;
Environment *env;

typedef struct Combo2 {
    Environment *env;
    TrainingInfo *info;
} combo2;

void custom_overlap(SDL_Renderer *renderer, SDL_Window *window) {
    env->agents[0]->brain->display(20, 380, 300, 200, renderer);
}

int main() {
    srand(time(NULL));
    std::string input;
    std::cout << "Agent requested? (-1 for best agent)" << std::endl;
    std::cin >> input;
    int agent_num = std::stoi(input);
    std::string agent_path;
    if(agent_num > -1) {
        agent_path = "data/best/agent_" + std::to_string(agent_num) + ".txt";
    } else {
        int highest_gen = 0;
        for (const auto& entry : std::filesystem::directory_iterator("data/best")) {
            if (entry.path().extension() == ".txt") {
                std::string filename = entry.path().stem().string();
                if (filename.substr(0, 6) == "agent_") {
                    int gen = std::stoi(filename.substr(6));
                    highest_gen = std::max(highest_gen, gen);
                }
            }
        }
        agent_path = "data/best/agent_" + std::to_string(highest_gen) + ".txt";
    }
    std::cout << "Agent path: " << agent_path << std::endl;
    std::string agent_str;

    // First, let's read the content from the file
    std::ifstream input_file(agent_path);
    if (input_file.is_open()) {
        std::stringstream buffer;
        buffer << input_file.rdbuf();
        agent_str = buffer.str();
        input_file.close();
        
        if (agent_str.empty()) {
            std::cout << "Warning: File is empty" << std::endl;
        } else {
            std::cout << "Successfully read agent data from file" << std::endl;
        }
    } else {
        std::cout << "Could not open file for reading" << std::endl;
        return 1;
    }

    // Now let's check if we actually have data to work with
    if (agent_str.empty()) {
        std::cout << "No agent data to process" << std::endl;
        return 1;
    }
    rend::RenderEngine renderer = rend::RenderEngine(800, 600, REND_INFOBOX);

    env = new Environment(1, defaultParameters());
    env->addToRenderer(&renderer);

    Agent *agent = env->agents[0];
    agent->brain->restore(agent_str);
    
    std::cout << "Agent restored" << std::endl;
       
    renderer.setCustomOverlapFunction(&(custom_overlap));
    renderer.setFPS(240);

    renderer.setZoomFactor(200);
    env->safety = &(renderer.lock);

    double time_elapsed_s = 0.0;
    double display_elapsed_s = 0.0;

    std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();

    while(renderer.handleEvents()) {
               
        if(renderer.isPaused()) {
            continue;
        }
        while(display_elapsed_s > time_elapsed_s) {
            env->system->process(env->params->dt, 1);
            time_elapsed_s += env->params->dt;
        }


        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        display_elapsed_s += env->params->update_dt;
        double time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - current).count();
        int time_to_sleep = (env->params->update_dt * 1000000 - time_spent);

        if(time_to_sleep > 0) {
            usleep(time_to_sleep);
        }

        agent->getInputs(env->system);
        agent->getOutputs();

        env->checkBaseData();

        agent->applyMovement(env->system, abs(agent->outputs[0]));
        agent->updateScore(env->system);

        current = std::chrono::steady_clock::now();
    }

    return 0;
}
