#pragma once

#include "../include/environment.hpp"


typedef struct TrainingInfo_s {
    bool change_best_agent;
    bool single_run;
    bool stop;
    int generation;
    int status;
    int best_agent;
    int num_cores;
    double current_time;
    double max_fitness;
    double avg_fitness;
    Environment *best_env;
    std::mutex lock;
} TrainingInfo;

typedef struct Combo {
    Environment *env;
    TrainingInfo *info;
    TrainingInfo *global_info;
} combo;

void generateMultiGroup(Environment *env, TrainingInfo *info);
void trainMultiCPU(combo *combos, int num_cores, TrainingInfo *info, Environment *env);
void train(Environment *env, TrainingInfo *info, TrainingInfo *global_info);
void nextBatch(Environment *env);
void resetupAgents(Environment *env);
void rankAgents(Environment *env);
void saveBestAgent(Environment *env);
void saveAgents(Environment *env);
void setupEnvironment(Environment *env);
void sanityCheck(Environment *env);
void logData(Environment *env, TrainingInfo *info);
