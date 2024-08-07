#pragma once

#define INPUTS_PER_BRANCHES 3
#define EXTRA_INPUTS 2
#define RESPONSE_SIZE 1

#define PENDULUM_SIZE 1
#define PENDULUM_MASS 1.0
#define PENDULUM_LENGTH 0.5

#define SPRING_CONSTANT 1000000
#define FRICTION_COEFF 0.2

#define TRAINING_TIME 30

#define X_BOUND 1

#define DT 1.0/2000.0 // In seconds
#define UPDATE_DT 1.0/60.0 // In seconds
                           //
// Mutations
#define SPLIT_CONNECTION_RATIO 0.05
#define ADD_CONNECTION_RATIO 0.8
#define RANDOM_AGENT_RATIO 0.05
#define MUT_COUNT 4
#define MUT_PROBA 0.25
#define NEW_VALUE_PROBA 0.2
#define WEIGHT_RANGE 1.0
#define BIAS_FULL_RANGE 1.0
#define BIAS_SMALL_RANGE 0.01
#define BIAS_FULL_PROBA 0.25
#define WEIGHT_FULL_RANGE 1.0
#define WEIGHT_SMALL_RANGE 0.01
#define WEIGHT_FULL_PROBA 0.75
#define ELITE_RATIO 0.1

#define BIAS_CHANGE_STRENGHT 1
#define BIAS_CHANGE_CHANCE 0.1

#define WEIGHT_CHANGE_STRENGHT 0.2
#define WEIGHT_CHANGE_CHANCE 0.1


typedef struct Parameters_s {
    int pendulum_size;
    double base_mass;
    double mass;
    double pendulum_length;
    double dt;
    double update_dt;
} Parameters;


