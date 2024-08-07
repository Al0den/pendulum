#include "../include/environment.hpp"
#include "../include/train.hpp"
#include "../include/utils.hpp"

#include <RenderEngine>

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <assert.h>

#define noRender false

TrainingInfo *info;
Environment *env;
rend::RenderEngine *renderer;

typedef struct Combo2 {
    Environment *env;
    TrainingInfo *info;
} combo2;

void custom_info_box(rend::InfoBox *info_box) {
    if(info->status != STATUS_TRAINING) return;
    info_box->setValue(0, 0, std::to_string(info->generation));
    info_box->setValue(0, 1, std::to_string(info->max_fitness).substr(0, std::to_string(info->max_fitness).find(".") + 3));
    info_box->setValue(1, 0, std::to_string(info->avg_fitness).substr(0, std::to_string(info->avg_fitness).find(".") + 3));
    info_box->setValue(1, 1, std::to_string(info->current_time).substr(0, std::to_string(info->current_time).find(".") + 3));
}

void custom_overlap(SDL_Renderer *renderer, SDL_Window *window) {
    if (info->status != STATUS_TRAINING) return;
    int h, w;
    SDL_GetWindowSize(window, &w, &h);
    env->agents[info->best_agent]->brain->display(20, h - 220, 300, 200, renderer);
}

void *startTraining(void *combo) {
    combo2 *comb = (combo2 *) combo;
    generateMultiGroup(comb->env, comb->info);
    return NULL;
}

int main() {
    srand(time(NULL));
    std::string input;
    std::cout << "Enter the number of agents: " << std::endl;
    std::cin >> input;
    int num_agents = std::stoi(input);

    
    std::cout << "Generation to load" << std::endl;
    std::cin >> input;
    int generation = std::stoi(input);

    env = new Environment(num_agents, defaultParameters());
    info = new TrainingInfo();
    info->single_run = false;

    if(generation != 0) {
        std::string path = "data/generations/generation_" + std::to_string(generation) + ".txt";
        std::ifstream file(path);

        if(file.is_open()) {
            std::string whole_file = "";
            std::string line;
            while(std::getline(file, line)) {
                whole_file += line;
                whole_file += "\n";
            }
            env->restore(whole_file);
            env->generation = generation;

            file.close();
        } else {
            std::cout << "Generation file not found" << std::endl;
        }
    }

    if(!noRender) {
        renderer = new rend::RenderEngine(800, 600, REND_INFOBOX);
        renderer->setCustomInfoBoxFunction(&(custom_info_box));
        renderer->setCustomOverlapFunction(&(custom_overlap));
        renderer->setFPS(240);

        renderer->setZoomFactor(200);
        env->safety = &renderer->lock;

        rend::InfoBox *info_box = renderer->getInfoBox();
        info_box->setRowCol(2, 2);
        info_box->setName(0, 0, "Generation"); 
        info_box->setName(0, 1, "Max Fitness");
        info_box->setName(1, 0, "Avg Fitness");
        info_box->setName(1, 1, "Current Time");
    } else {
        env->safety = new std::mutex();
    }

    info->num_cores = 8;
    info->best_env = env;
    
    combo2 *comb = new combo2();
    comb->env = env;
    comb->info = info;
    pthread_t thread;
    pthread_create(&thread, NULL, startTraining, (void *) comb);

    if(!noRender) {
        while(renderer->handleEvents()) {
            if(info->change_best_agent && info->status == STATUS_TRAINING) {
                Agent* agent = info->best_env->agents[info->best_agent];
                info->change_best_agent = false;
                
                renderer->clearObjects();

                double ball_radius = 0.05;

                RigidBody *prev = nullptr;

                for(int i=0; i<info->best_env->params->pendulum_size + 1; i++) {
                    RigidBody *body = info->best_env->system->getRigidBody(agent->head_id + i);
                    auto obj = std::make_unique<rend::BallRenderer>(&(body->p_x), &(body->p_y), ball_radius, 255, 255, 255, 0);
                    renderer->attachObject(std::move(obj)); 
                    if(prev != nullptr) {
                        double dist = sqrt((body->p_x - body->p_x) * (body->p_x - body->p_x) + (body->p_y - body->p_y) * (body->p_y - body->p_y));
                        auto new_obj = std::make_unique<rend::SpringRenderer>(&(body->p_x), &(body->p_y), &(prev->p_x), &(prev->p_y), dist, 255, 255, 255, 0);
                        renderer->attachObject(std::move(new_obj));
                    }
                    prev = body;
                }
            }
            usleep(10);
        }
    } else {
        while(true) {
            usleep(100000);
        }
    }
}
