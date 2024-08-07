#include "../include/environment.hpp"
#include "../include/utils.hpp"

#include <RenderEngine>

#include <unistd.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <assert.h>

Environment *env;
bool pressed = false;

int keyInfo[4] = {0, 0, 0, 0};

void custom_overlap(SDL_Renderer *renderer, SDL_Window *window) {
    env->agents[0]->brain->display(20, 380, 300, 200, renderer);
}

void customEventHandler(SDL_Event *e) {
    if(e->type == SDL_KEYDOWN) {
        int rand_connection = rand() % env->agents[0]->brain->connections.size();
        switch (e->key.keysym.sym) {
            case SDLK_m:
                env->agents[0]->brain->splitConnection(env->agents[0]->brain->connections[rand_connection]);
                break;
            case SDLK_n:
                env->agents[0]->brain->removeConnection(env->agents[0]->brain->connections[rand_connection]);
                break;
            case SDLK_a:
                keyInfo[0] = 1;
                break;
            case SDLK_d:
                keyInfo[1] = 1;
                break;
        }
    } else if(e->type == SDL_KEYUP) {
        switch (e->key.keysym.sym) {
            case SDLK_a:
                keyInfo[0] = 0;
                break;
            case SDLK_d:
                keyInfo[1] = 0;
                break;
        }
    }
}

int main() {
    srand(time(nullptr));
    env = new Environment(1, defaultParameters());
    rend::RenderEngine renderer = rend::RenderEngine(800, 600, REND_INFOBOX);
    rend::InfoBox *info_box = renderer.getInfoBox();

    renderer.setCustomOverlapFunction(&(custom_overlap));
    renderer.setCustomSDLEventHandler(&(customEventHandler));

    env->addToRenderer(&renderer);

    info_box->setRowCol(2, 2);

    double time_elapsed_s = 0.0;
    double display_elapsed_s = 0.0;

    std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();

    Agent *agent = env->agents[0];
    agent->outputs = new double[1];

    std::string path1 = "data/test1.txt";
    std::string path2 = "data/test2.txt";

    std::string serial1 = agent->brain->serialize();
    agent->brain->restore(serial1);
    std::string serial2 = agent->brain->serialize();

    assert(serial1 == serial2);
    
    //Save to path1 and path2

    std::ofstream file1(path1);
    file1 << serial1;

    std::ofstream file2(path2);
    file2 << serial2;


    file1.close();
    file2.close();

    agent->brain->restore(agent->brain->serialize());
    agent->brain->splitConnection(agent->brain->connections[0]);
    agent->brain->restore(agent->brain->serialize());
    
    while(renderer.handleEvents()) {
        if(renderer.isPaused()) {
            continue;
        }
        while (display_elapsed_s > time_elapsed_s) {
            env->system->process(env->params->dt, 1);
            time_elapsed_s += env->params->dt;
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        display_elapsed_s += env->params->update_dt;

        double time_spent = std::chrono::duration_cast<std::chrono::microseconds>(end - current).count() / 1000000.0;

        int time_to_sleep = (env->params->update_dt * 1000000 - time_spent);

        if(time_to_sleep > 0) {
            usleep(time_to_sleep);
        }

        agent->outputs = new double[1];
        
        agent->getInputs(env->system);
        //agent->getOutputs();

        if(keyInfo[0] == 1) {
            agent->outputs[0] = -1;
        } else if(keyInfo[1] == 1) {
            agent->outputs[0] = 1;
        } else {
            agent->outputs[0] = 0;
        }

        env->checkBaseData();

        agent->applyMovement(env->system, abs(agent->outputs[0]));
        agent->updateScore(env->system);

        current = std::chrono::steady_clock::now();
    }
    return 0;
}
