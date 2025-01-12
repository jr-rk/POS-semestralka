#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "initData.h"
#include "sharedData.h"


//nastavenie hodnot
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
} Object;

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

char* get_direction(int direction);

void update_world_trajectory(SimulationState *state);

void update_world_summary(SimulationState *state);

void update_world(SimulationState *state);

void place_obstacle(SimulationState *state);

void init_simulation(SimulationState *state, WalkerState *walker, InitData *data);

void destroy_simulation(SimulationState *state);

void print_world(SimulationState *state);

int choose_direction(const double probabilities[], int size);

void single_walk(SimulationState *state ,int start_x, int start_y);

bool is_obstacle(SimulationState *state, int x, int y);

void shared_memory_write(SimulationState *state, SharedData *data, int walker_x, int walker_y, int current_rep, int done);

void simulate_walk(SimulationState *state, SharedData *shm_data);

void calculate_probability_to_reach_center(SimulationState *state, SharedData *shm_data);

void zapis_do_suboru(SimulationState *state);

void print_usage(const char *prog_name);

void create_new_simulation(char *argv[]);

int connect_to_existing_simulation( char *argv[]);

#endif // !SERVER_H
