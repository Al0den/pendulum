#include "../include/train.hpp"
#include "../include/config.hpp"
#include "../include/utils.hpp"

#include <assert.h>
#include <fstream>
#include <iostream>

void *train_func(void *c) {
    combo *combo_input = (combo*)c;
    train(combo_input->env, combo_input->info, combo_input->global_info);
    return NULL;
}


void generateMultiGroup(Environment *env, TrainingInfo *info) {
    assert (info != nullptr);
    assert (env != nullptr);
    assert (info->num_cores > 0);
    
    std::cout << "Generating training combos" << std::endl;
    combo *combos = new combo[info->num_cores];
    int num_agents_per_core = env->agents.size() / info->num_cores;
    int remaining_agents = env->agents.size() % info->num_cores;

    assert (remaining_agents == 0);

    for (int i=0; i<info->num_cores; i++) {
        Environment *new_env = new Environment(num_agents_per_core, env->params);
        new_env->generation = env->generation;
        for(int j=0; j<num_agents_per_core; j++) {
            delete new_env->agents[j];
            new_env->agents[j] = env->agents[i * num_agents_per_core + j];
            new_env->agents[j]->head_id = j * (env->params->pendulum_size + 1);
        }
        TrainingInfo *new_info = new TrainingInfo;
        new_info->num_cores = info->num_cores;
        new_info->generation = env->generation;
        new_info->single_run = true;

        combos[i].env = new_env;
        combos[i].info = new_info;
        combos[i].global_info = info;
    }
    info->stop = false;

    std::cout << "Generated " << info->num_cores << " combos, each containing " << num_agents_per_core << " agents." << std::endl;

    trainMultiCPU(combos, info->num_cores, info, env);
}

void deleteCombos(combo *combos, int num_cores) {
    for(int i=0; i<num_cores; i++) {
        delete combos[i].env->safety;
        delete combos[i].info;
    }
}

void trainMultiCPU(combo *combos, int num_cores, TrainingInfo *info, Environment *env) {
    pthread_t *threads = new pthread_t[num_cores];
    std::cout << "-------- Training Generation " << env->generation << " --------" << std::endl;
    std::cout << "Training " << env->agents.size() << " agents on " << num_cores << " cores..." << std::endl;
    info->max_fitness = 0.0;
    info->avg_fitness = 0.0;
    info->current_time = 0.0;
    info->best_agent = 0;
    info->change_best_agent = true;
    info->generation = env->generation;
    info->status = STATUS_TRAINING;
    for(int i=0; i<num_cores; i++) {
        combos[i].env->generation = env->generation;
        combos[i].info->single_run = true;
        combos[i].info->status = STATUS_UNTRAINED;
        combos[i].env->status = STATUS_UNTRAINED;
        pthread_create(&threads[i], NULL, train_func, (void*)&combos[i]);
    }
    for(int i=0; i<num_cores; i++) {
        pthread_join(threads[i], NULL);
    }
    delete[] threads;
    
    env->status = STATUS_TRAINED;
    info->status = STATUS_TRAINED;

    logData(env, info);
    rankAgents(env);

    info->status = env->status;

    nextBatch(env);

    info->status = env->status;

    saveAgents(env);

    info->status = STATUS_UNTRAINED;
    env->generation += 1;
    std::cout << "Best Fitness: " << info->max_fitness << ", Average Fitness: " << info->avg_fitness << std::endl;

    deleteCombos(combos, num_cores);
    generateMultiGroup(env, info);
}

void train(Environment *env, TrainingInfo *info, TrainingInfo *global_info) {
    assert (env != nullptr);
    assert (info != nullptr);
    assert (env->status == STATUS_UNTRAINED);
    assert (env->params->dt < env->params->update_dt);
    
    setupEnvironment(env);
    sanityCheck(env);

    env->status = STATUS_TRAINING;

    info->status = STATUS_TRAINING;
    info->generation = env->generation;
    info->avg_fitness = 0.0;
    info->max_fitness = 0.0;
    info->current_time = 0.0;
    info->best_agent = 0;
    info->change_best_agent = true;
    
    double simulation_s = 0.0;
    double next_input_s = 0.0;

    int n = env->agents.size();
    double prev_fitness = 0.0;
    while(simulation_s < TRAINING_TIME) {
        while(simulation_s < next_input_s) {
            env->system->process(env->params->dt, 1);
            simulation_s += env->params->dt;
        }
        info->avg_fitness = 0;
        for(int i=0; i<n; i++) {
            env->agents[i]->updateScore(env->system);
            env->agents[i]->getInputs(env->system);
            env->agents[i]->getOutputs();
            env->agents[i]->applyMovement(env->system, abs(env->agents[i]->outputs[0]));
            info->avg_fitness += env->agents[i]->fitness / n;

            global_info->lock.lock();
            if(env->agents[i]->fitness > global_info->max_fitness) {
                global_info->max_fitness = env->agents[i]->fitness;
                global_info->best_agent = i;
                global_info->change_best_agent = true;
                global_info->best_env = env;
                global_info->current_time = simulation_s;
            }
            global_info->lock.unlock();
        }
        double fitness = info->avg_fitness / global_info->num_cores;
        global_info->avg_fitness -= prev_fitness;
        global_info->avg_fitness += fitness;
        prev_fitness = fitness;

        info->current_time = simulation_s;

        next_input_s += env->params->update_dt;
    }
    return;
}

void rankAgents(Environment *env) {
    assert (env->status == STATUS_TRAINED);
    std::cout << "Ranking " << env->agents.size() << " agents...";
    std::vector<Agent*> sorted_agents;

    for(int i=0; i<env->agents.size(); i++) {
        int n_sorted = sorted_agents.size();
        bool sorted = false;
        for(int j=0; j<n_sorted; j++ ) {
            if(env->agents[i]->fitness > sorted_agents[j]->fitness) {
                sorted_agents.insert(std::next(sorted_agents.begin(), j), env->agents[i]);
                sorted = true;
                break;
            }
        }
        if(!sorted) {
            sorted_agents.push_back(env->agents[i]);
        }
    }

    assert (sorted_agents.size() == env->agents.size());
    for(int i=0; i<sorted_agents.size() - 1; i++) {
        assert (sorted_agents[i]->fitness >= sorted_agents[i+1]->fitness);
    }

    env->agents = sorted_agents;
    env->status = STATUS_RANKED;
    std::cout << "\rRanked " << env->agents.size() << " agents    " << std::endl;
}

Agent *weightedSelection(Environment *env, double *weights) {
    RandomDoubleGenerator rdg(0, 1);
    double rand_choice = rdg();
    int i=0;
    double sum = weights[0];
    while(sum < rand_choice && i < env->agents.size() - 2) {
        sum += weights[i+1];
        i++;
    }
    if(sum < rand_choice) {
        i = env->agents.size() - 1;
    }
    return env->agents[i];
}

void nextBatch(Environment *env) {
    std::cout << "Generating next agents...";
    assert (env->status == STATUS_RANKED);
    std::vector<Agent*> new_batch;

    int identical_copies = max((int)(env->agents.size() * ELITE_RATIO), 1);
    
    int j = 0;

    for(int i=0; i < identical_copies; i++) {
        Agent *new_agent = new Agent(j * (env->params->pendulum_size + 1), env->params->pendulum_size, env->params);
        new_agent->brain->restore(env->agents[i]->brain->serialize());
        new_batch.push_back(new_agent);
        j++;
    }

    double *weights = new double[env->agents.size()];
    double total = 0.0;

    for(int i=0; i<env->agents.size(); i++) {
        weights[i] = env->agents[i]->fitness;
        total += env->agents[i]->fitness;
    }

    for(int i=0; i<env->agents.size(); i++) {
        weights[i] /= total;
    }
    RandomDoubleGenerator rdg(0, 1);
    
    std::vector<std::string> serialized_agents;
    
    while(new_batch.size() < env->agents.size()) {
        Agent* chosen_agent_old = weightedSelection(env, weights);
        Agent* chosen_agent = new Agent(j * (env->params->pendulum_size + 1), env->params->pendulum_size, env->params);
        if(rdg() < RANDOM_AGENT_RATIO) {
        } else {
            chosen_agent->brain->restore(chosen_agent_old->brain->serialize()); 
            chosen_agent->brain->mutateGenome();
        }
        // Make sure agent isnt already in the batch
        std::string serial = chosen_agent->brain->serialize();
        if (std::find(serialized_agents.begin(), serialized_agents.end(), serial) != serialized_agents.end()) {
            delete chosen_agent;
            continue;
        }
        new_batch.push_back(chosen_agent);
        j++;
    }
    
    for(int i=0; i<env->agents.size(); i++) {
        delete env->agents[i];
    }
    delete[] weights;

    env->agents = new_batch;
    env->status = STATUS_MUTATED;
    std::cout << "\rGenerated next agents    " << std::endl;
}

void saveAgents(Environment *env) {
    std::cout << "Saving generated agents...";
    assert (env->status = STATUS_MUTATED);
    std::string serial = env->serialize();
    std::string path = "data/generations/generation_" + std::to_string(env->generation + 1) + ".txt";

    std::ofstream file(path);
    if(!file.is_open()) {
        std::cerr << "Pas reussis a ouvrir le fichier pour sauvegarder les agents" << std::endl;
        return;
    }
    file << serial;
    file.close();
    std::cout << "\rSaved generated agents    " << std::endl;
}

void setupEnvironment(Environment *env) {
    env->safety->lock();
    assert (env->status == STATUS_UNTRAINED);
    RandomDoubleGenerator rdg(0, M_PI * 2);
    env->createSystem();
    for(int i=0; i<env->agents.size(); i++) {
        Agent *agent = env->agents[i];
        agent->inputs = nullptr;
        agent->outputs = nullptr;
        agent->fitness = 0;
        for(int i=0; i<agent->num_branches; i++) {
            double theta = rdg();
            RigidBody *begin = env->system->getRigidBody(agent->head_id + i);
            RigidBody *end = env->system->getRigidBody(agent->head_id + i + 1);
            end->p_x = begin->p_x + env->params->pendulum_length * cos(theta);
            end->p_y = begin->p_y + env->params->pendulum_length * sin(theta);
        }
    }
    
    env->safety->unlock();
}

void sanityCheck(Environment *env) {
    return;
    env->safety->lock();
    for(int i=0; i<env->agents.size(); i++) {
        env->agents[i]->brain->updateLayers();
        for(int j=0; j<env->agents.size(); j++) {
            if (i != j) {
                assert (env->agents[i]->head_id != env->agents[j]->head_id);
            }
        }
        for(int j=0; j<env->agents[i]->brain->connections.size(); j++) {
            Connection *connection = env->agents[i]->brain->connections[j];
            assert (((Node *)(connection->from))->layer_id < ((Node*)(connection->towards))->layer_id);
        }
        assert (env->agents[i]->brain->nodes.size() >= INPUTS_PER_BRANCHES * env->params->pendulum_size + EXTRA_INPUTS);
    }
    env->safety->unlock();
    assert(env->system->getRigidBodiesCount() == (env->params->pendulum_size + 1) * env->agents.size());
}

void logData(Environment *env, TrainingInfo *info) {
    std::string log_line = "";
    log_line += std::to_string(info->generation) + ",";
    log_line += std::to_string(info->max_fitness) + ",";
    log_line += std::to_string(info->avg_fitness) + ",";
    log_line += std::to_string(env->agents[info->best_agent]->brain->nodes.size()) + ",";
    log_line += std::to_string(env->agents[info->best_agent]->brain->connections.size()) + ",";
    double avg_net_size = 0;
    double avg_net_connections = 0;
    for(Agent *agent : env->agents) {
        avg_net_size += agent->brain->nodes.size();
        avg_net_connections += agent->brain->connections.size();
    }
    avg_net_size /= env->agents.size();
    avg_net_connections /= env->agents.size();
    log_line += std::to_string(avg_net_size) + ",";
    log_line += std::to_string(avg_net_connections);
    // Check if log file exists
    std::ifstream file("data/logs/log.csv");
    if (!file.is_open()) {
        std::ofstream new_file("data/logs/log.csv");
        new_file << "Generation,Max Fitness,Avg Fitness,Nodes,Connections,Avg net size, Avg Connections\n";
        new_file.close();
    }

    std::ofstream log_file("data/logs/log.csv", std::ios_base::app);
    log_file << log_line << "\n";
    log_file.close();
}
