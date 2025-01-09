//
// Created by maros on 8. 1. 2025.
//

#ifndef WORLD_H
#define WORLD_H
#include "initData.h"
#define MOVE_UP " ^^ "
#define MOVE_DOWN " VV "
#define MOVE_LEFT " << "
#define MOVE_RIGHT " >> "
#define WALKER " CH "
#define OBSTACLE " PP "
#define SPACE " .. "
#define CENTER_WORLD " SS "
#define MAX_STEPS 30

typedef struct Object {
    int x;
    int y;
    int direction;
}Object;

typedef struct WalkerState {
    int x;
    int y;
    //int direction; //kde sa pomohol -> 0 - up | 1 - down | 2 - right | 3 - left sluzi pre kreslenie trajektorie chodca
    int step;
    bool centerReached;
    Object trajectory[MAX_STEPS];
} WalkerState;

typedef struct SimulationState {
    int world_width_;
    int world_height_;
    int max_steps_; //hodnota K
    double probabilities[4]; // 0 - up | 1 - down | 2 - right | 3 - left
    int num_replications;
    bool obstaclesAllowed;
    bool interactive_mode;
    bool summary_mode;
    int obstacleCount;
    char ***world;
    double **prob_world;
    Object *obstacles;
    WalkerState *walker;
} SimulationState;

void init_walker(WalkerState *walker);
void init_simulation(SimulationState *state, WalkerState *walker, initData *data);
void destroy_simulation(SimulationState *state);
void update_world_trajectory(SimulationState *state);
void update_world_summary(SimulationState *state);
void update_world(SimulationState *state);
void place_obstacle(SimulationState *state);
void print_world(SimulationState *state); // toto bude mat asi client
int choose_direction(double probabilities[], int size);
char* get_direction(int direction);


void single_walk(SimulationState *state ,int start_x, int start_y);
bool is_obstacle(SimulationState *state, int x, int y);
void simulate_walk(SimulationState *state);
void calculate_probability_to_reach_center(SimulationState *state);

#endif //WORLD_H
