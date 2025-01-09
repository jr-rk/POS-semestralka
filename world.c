//
// Created by maros on 8. 1. 2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#include "world.h"
//#include "initData.h"

void init_walker(WalkerState *walker) {
    walker->centerReached = false;
    walker->step = 0;
    walker->x = 0;
    walker->y = 0;
}

char* get_direction(int direction) {
    char* result = malloc(5*sizeof(char));
    if (!result) {
        perror("allocation failed");
        exit(EXIT_FAILURE);
    }
    switch (direction) {
        case 0: strcpy(result, MOVE_RIGHT); break;
        case 1: strcpy(result, MOVE_LEFT); break;
        case 2: strcpy(result, MOVE_DOWN); break;
        case 3: strcpy(result, MOVE_UP); break;
        default: strcpy(result, SPACE); break;
    }
    return result;
}

void update_world_trajectory(SimulationState *state) {
    int center_row = state->world_width_ / 2;
    int center_col = state->world_height_ / 2;
    for (int i = 0; i < state->world_width_; i++) {
        for (int j = 0; j < state->world_height_; j++) {
            snprintf(state->world[i][j], 10, SPACE); //vsetky bunky su prednastavene na space

            if (state->interactive_mode) {
                // Trajektoria chodca
                for (int k = 0; k < (state->walker->step); k++) {
                    if ((state->walker->trajectory[k].x == i && state->walker->trajectory[k].y == j) &&
                        !(i == state->walker->x && j == state->walker->y)
                    ) {
                        char* usedDirection = get_direction(state->walker->trajectory[k].direction);
                        snprintf(state->world[i][j], 10, usedDirection);
                        free(usedDirection);
                    }
                }
            }

            if (i == state->walker->x && j == state->walker->y) {
                //aktualna pozicia chadca
                snprintf(state->world[i][j], 10, WALKER);
            } else if (i == center_row && j == center_col) {
                //stred sveta
                snprintf(state->world[i][j], 10, CENTER_WORLD);
            }else if (state->obstaclesAllowed) {
                //prekazky
                for (int k = 0; k < state->obstacleCount; k++) {
                    if (state->obstacles[k].x == i && state->obstacles[k].y == j) {
                        snprintf(state->world[i][j], 10, OBSTACLE);
                        break;
                    }
                }
            }


        }
    }
}

void update_world_summary(SimulationState *state) {
    int center_row = state->world_width_ / 2;
    int center_col = state->world_height_ / 2;
    for (int i = 0; i < state->world_width_; i++) {
        for (int j = 0; j < state->world_height_; j++) {
            snprintf(state->world[i][j], 10, SPACE); // prednastavene na space

            if (state->summary_mode) {
                 if (state->prob_world[i][j] != 0.0) {
                    //pravdepodobnosti
                    snprintf(state->world[i][j], 10, "%.2f", state->prob_world[i][j]);
                } else if (i == center_row && j == center_col) {
                    //stred sveta
                    snprintf(state->world[i][j], 10, CENTER_WORLD);
                }else if (state->obstaclesAllowed) {
                    //prekazky
                    for (int k = 0; k < state->obstacleCount; k++) {
                        if (state->obstacles[k].x == i && state->obstacles[k].y == j) {
                            snprintf(state->world[i][j], 10, OBSTACLE);
                            break;
                        }
                    }
                }

            }
        }
    }
}
void update_world(SimulationState *state) {
    if (state->interactive_mode) {
        update_world_trajectory(state);
    } else {
        update_world_summary(state);
    }
}

void place_obstacle(SimulationState *state) {
    int center_x = state->world_width_ / 2;
    int center_y = state->world_height_ / 2;

    for (int i = 0; i < state->obstacleCount; i++) {
        int x, y;

        do {
            x = rand() % state->world_width_;
            y = rand() % state->world_height_;
        } while ((x == center_x && y == center_y) || strcmp(state->world[x][y], OBSTACLE) == 0 || strcmp(state->world[x][y], WALKER) == 0);
        state->obstacles[i].x = x;
        state->obstacles[i].y = y;
    }

    update_world(state);
}

void init_simulation(SimulationState *state, WalkerState *walker, initData *data) {
    state->world = malloc(data->width * sizeof(char **));
    state->prob_world = (double**)malloc(data->width * sizeof(double *));
    state->world_width_ = data->width;
    state->world_height_ = data->height;
    state->max_steps_ = data->max_steps;
    state->probabilities[0] = data->probabilities[0];
    state->probabilities[1] = data->probabilities[1];
    state->probabilities[2] = data->probabilities[2];
    state->probabilities[3] = data->probabilities[3];
    state->num_replications = data->num_reps;
    state->interactive_mode = false;
    state->summary_mode = true;
    state->obstaclesAllowed = (data->obstaclesAllowed == 1 ? true : false); // ak si uzivatel vybere svet s prekazkami tak TRUE
    state->obstacleCount = (int)(data->width * data->height) * (0.2);
    state->obstacles = malloc(state->obstacleCount * sizeof(Object));
    state->walker = walker;

    for (int i = 0; i < state->max_steps_; i++) {
        state->walker->trajectory[i].x = -1;
        state->walker->trajectory[i].y = -1;
        state->walker->trajectory[i].direction = -1;
    }

    //init world interaktivny rezim
    for (int i = 0; i < state->world_width_; i++) {
        state->world[i] = malloc(state->world_height_ * sizeof(char *));
        for (int j = 0; j < state->world_height_; j++) {
            state->world[i][j] = malloc(10 * sizeof(char)); // Pre ulozenie textu
            snprintf(state->world[i][j], 10, SPACE);
        }
    }

    //init world na pravdepodobnosti
    for (int i = 0; i < state->world_width_; i++) {
        state->prob_world[i] = malloc(data->width * sizeof(double)); //alokacia kazdeho riadku
        for (int j = 0; j < state->world_height_; j++) {
            state->prob_world[i][j] = 0.0;
        }
        //mozme pridat podmienku ak alokovanie zlyhalo
        if (state->prob_world[i] == NULL) {
            fprintf(stderr, "Allocation failed for row %d\n", i);
            exit(1);
        }
    }



    //naplnenie pola
    update_world(state);

    if (state->obstaclesAllowed) {
        place_obstacle(state);
    }
}

void destroy_simulation(SimulationState *state) {
    if (state->obstaclesAllowed && state->obstacles != NULL) {
        free(state->obstacles);
        state->obstacles = NULL;
    }
    //uvolnenie pamate world
    for (int i = 0; i < state->world_width_; i++) {
        for (int j = 0; j < state->world_height_; j++) {
            free(state->world[i][j]);
        }
        free(state->world[i]);
    }
    free(state->world);

    //ovolnenie pamete prob_world
    for (int i = 0; i < state->world_width_; i++) {
        free(state->prob_world[i]);
    }
    free(state->prob_world);
}

void print_world(SimulationState *state) {
    printf("     ");
    for (int j = 0; j < state->world_height_; j++) {
        if (j < 10) {
            printf(" 0%d  ", j);
        } else {
            printf(" %d  ", j);
        }
    }
    printf("\n");
    for (int i = 0; i < state->world_width_; i++) {
        printf("%3d |", i);
        for (int j = 0; j < state->world_height_; j++) {
            printf("%s ", state->world[i][j]);
        }
        printf("\n");
    }
}

int choose_direction(double probabilities[], int size) {
    double random_number = ((double) rand() / (double) RAND_MAX);
    double cumulative = 0.0;
    for (int i = 0; i < size; i++) {
        cumulative += probabilities[i];
        if (random_number < cumulative) {
            return i;
        }
    }
}

void single_walk(SimulationState *state ,int start_x, int start_y) {
    WalkerState *walker = state->walker;
    walker->x = start_x;
    walker->y = start_y;
    walker->step = 0;
    walker->centerReached = false;
    int center_x = state->world_width_ / 2;
    int center_y = state->world_height_ / 2;

    printf("START WALKING, Walker on position [%d, %d] \n", walker->x, walker->y);
    update_world(state);
    print_world(state);

    for (int step = 0; step < state->max_steps_; step++) {
        if (walker->centerReached == false) {
            int direction = choose_direction(state->probabilities, 4);
            int new_x = walker->x;
            int new_y = walker->y;
            switch (direction) {
                case 0:
                    new_y = (walker->y + 1) % state->world_height_;
                    walker->trajectory[step].x = walker->x;
                    walker->trajectory[step].y = walker->y;
                    walker->trajectory[step].direction = direction;
                    break; //up
                case 1:
                    new_y = (walker->y - 1 + state->world_height_) % state->world_height_;
                    walker->trajectory[step].x = walker->x;
                    walker->trajectory[step].y = walker->y;
                    //walker->direction = direction;
                    walker->trajectory[step].direction = direction;
                    break; //down
                case 2:
                    new_x = (walker->x + 1) % state->world_width_;
                    walker->trajectory[step].x = walker->x;
                    walker->trajectory[step].y = walker->y;
                    //walker->direction = direction;
                    walker->trajectory[step].direction = direction;
                    break; // right
                case 3:
                    new_x = (walker->x - 1 + state->world_width_) % state->world_width_;
                    walker->trajectory[step].x = walker->x;
                    walker->trajectory[step].y = walker->y;
                    //walker->direction = direction;
                    walker->trajectory[step].direction = direction;
                    break; //left
            }
            if (state->obstaclesAllowed) {
                //aby chodec nestupil na prekazku
                if (strcmp(state->world[new_x][new_y], OBSTACLE) != 0) {
                    walker->x = new_x;
                    walker->y = new_y;
                } else {
                    printf("KROK: %d - Chodec sa pokusil stupit na prekazku[%d, %d] - nemozes \n", step, new_x, new_y);
                } //- toto asi tiez mozes vymazat
            } else {
                walker->x = new_x;
                walker->y = new_y;
            }
            walker->step++;

            if (walker->x == center_x && walker->y == center_y) {
                printf("--------------------------------------\n");
                printf("Step %d: WALKER[%d, %d] DOSIAHOL STRED V %d KROKOCH\n",step+1, walker->x, walker->y, step+1);
                if (state->interactive_mode) {
                    update_world(state);
                }
                //print_world(state);
                walker->centerReached = true;
                break;
            }
        }

        if (state->interactive_mode) {
            printf("--------------------------------------\n");
            printf("Step %d: Walker is on position [%d, %d] ->", walker->step, walker->x, walker->y);
            printf(" Stred sveta: [%d, %d] \n ", center_x, center_y);
            update_world(state);
            //print_world(state);
        }

        if (state->summary_mode) {
            printf("--------------------------------------\n");
            printf("Step %d: Walker is on position [%d, %d] -> ", walker->step, walker->x, walker->y);
            printf(" Stred sveta: [%d, %d] \n ", center_x, center_y);
            update_world(state);
            //print_world(state);
        }
    }


}

bool is_obstacle(SimulationState *state, int x, int y) {
    for (int k = 0; k < state->obstacleCount; k++) {
        if (state->obstacles[k].x == x && state->obstacles[k].y == y) {
            return true; // Prekazka sa nachadza na suradniciach x, y
        }
    }

    return false; //prekazka neni na suradniciach (x, y)
}

void simulate_walk(SimulationState *state) {
    //WalkerState *walker = state->walker;
    int center_x = state->world_width_ / 2;
    int center_y = state->world_height_ / 2;

    for (int x = 0; x < state->world_width_; x++) {
        for (int y = 0; y < state->world_height_; y++) {
            if (!is_obstacle(state, x, y) && !(x == center_x && y == center_y)) {
                for (int t = 0; t < state->num_replications; t++) {
                    printf("******************************************************* \n ");
                    printf("ZACIATOK REPLIKACIE %d \n ", t + 1);
                    single_walk(state, x, y);
                    print_world(state);
                }
            }
            else {
                printf("Preskakuje sa bod [%d][%d], pretoze je to prekazka.\n", x, y);
            }
        }
    }
}



void calculate_probability_to_reach_center(SimulationState *state) {
    WalkerState *walker = state->walker;
    int center_x = state->world_width_ / 2;
    int center_y = state->world_height_ / 2;
    for (int x = 0; x < state->world_width_; x++) {
        for (int y = 0; y < state->world_height_; y++) {

            //aby nerobil replikacie tam kde su prekazky a stred
             if (!is_obstacle(state, x, y) && !(x == center_x && y == center_y)) {
                int succesful_reaches_center = 0;

                for (int t = 0; t < state->num_replications; t++) {
                    printf("******************************************************* \n ");
                    printf("ZACIATOK REPLIKACIE %d \n ", t + 1);
                    single_walk(state, x, y);
                    // podmienka ak dosiahol stred
                    if (walker->x == center_x && walker->y == center_y) {
                        succesful_reaches_center += 1;
                    }
                }
                double probability = (double)succesful_reaches_center / (double)state->num_replications;
                //zapisat do matice cislo pravdepodobnosti.
                state->prob_world[x][y] = probability;
                update_world(state);
                print_world(state);
            } else {
                printf("Preskakuje sa bod [%d][%d], pretoze je to prekazka.\n", x, y);
            }

        }
    }
}