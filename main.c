#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

// world_with - world_height -> dostanem od klienta
// max_steps -> dostanem od klienta
//probabilities -> tiez dostanem od klienta
//num_of_replications -> tiez od klienta
//svet bez prekazok, svet s prekazkami
//interaktivny mod, summarny mod
//subor to este neni implementovane

//zatial nastavim tieto hodnoty rucne
#define MOVE_UP " ^^ "
#define MOVE_DOWN " VV "
#define MOVE_LEFT " << "
#define MOVE_RIGHT " >> "
#define WALKER " CH "
#define OBSTACLE " PP "
#define SPACE " .. "
#define CENTER_WORLD " SS "
#define WORLD_WIDTH 5
#define WORLD_HEIGHT 5
#define MAX_STEPS 30
#define PROBABILITIES 0.25
#define NUM_REPLICATIONS 3
#define OBSTACLE_COUNT 5


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

void init_simulation(SimulationState *state, WalkerState *walker, bool obstaclesAllowed, bool interactiveMode, bool summaryMode) {
    state->world = malloc(WORLD_WIDTH * sizeof(char **));
    state->prob_world = (double**)malloc(WORLD_WIDTH * sizeof(double *));
    state->world_width_ = WORLD_WIDTH;
    state->world_height_ = WORLD_HEIGHT;
    state->max_steps_ = MAX_STEPS;
    state->probabilities[0] = PROBABILITIES;
    state->probabilities[1] = PROBABILITIES;
    state->probabilities[2] = PROBABILITIES;
    state->probabilities[3] = PROBABILITIES;
    state->num_replications = NUM_REPLICATIONS;
    state->interactive_mode = interactiveMode;
    state->summary_mode = summaryMode;
    state->obstaclesAllowed = obstaclesAllowed; // ak si uzivatel vybere svet s prekazkami tak TRUE
    state->obstacleCount = OBSTACLE_COUNT;
    state->obstacles = malloc(OBSTACLE_COUNT * sizeof(Object));
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
        state->prob_world[i] = malloc(WORLD_WIDTH * sizeof(double)); //alokacia kazdeho riadku
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

    if (obstaclesAllowed) {
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

void write_to_file(const char *filename, SimulationState *state) {
    // printf("World dimensions: %d x %d\n", state->world_width_, state->world_height_);
    // printf("Number of replications: %d\n", state->num_replications);
    // printf("Max steps: %d\n", state->max_steps_);
    // printf("Probabilities: %.2f, %.2f, %.2f, %.2f\n",
    //        state->probabilities[0], state->probabilities[1],
    //        state->probabilities[2], state->probabilities[3]);


    // for (int i = 0; i < state->world_width_; i++) {
    //     for (int j = 0; j < state->world_height_; j++) {
    //         printf("World[%d][%d]: %s\n", i, j, state->world[i][j]);
    //     }
    // }

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    printf("File opened successfully\n");

    //Zapis rozmerov sveta
    fprintf(file, "%d %d\n", state->world_width_, state->world_height_);

    // Zapis poctu replikacii
    fprintf(file, "%d\n", state->num_replications);

    // Zápis počtu krokov
    fprintf(file, "%d\n", state->max_steps_);

    // Zápis pravdepodobností pohybu
    fprintf(file, "%.2f %.2f %.2f %.2f\n", state->probabilities[0], state->probabilities[1], state->probabilities[2], state->probabilities[3]);

    // Zapis modu 1 - interactive mode | 0 - summary mode
    fprintf(file, "%d\n", state->interactive_mode);

    // Zápis sveta
    for (int i = 0; i < state->world_width_; i++) {
        for (int j = 0; j < state->world_height_; j++) {
            fprintf(file, " %s ", state->world[i][j]);
        }
        fprintf(file, "\n"); // nový riadok pre ďalší rad sveta
    }

    // Zatvorenie súboru
    fclose(file);
}

void read_from_file(const char *filename, SimulationState *state) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    // Čítanie rozmerov sveta
    if (fscanf(file, "%d %d\n", &state->world_width_, &state->world_height_) != 2) {
        fprintf(stderr, "Error reading world dimensions\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Čítanie počtu replikácií
    if (fscanf(file, "%d\n", &state->num_replications) != 1) {
        fprintf(stderr, "Error reading number of replications\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Čítanie počtu krokov
    if (fscanf(file, "%d\n", &state->max_steps_) != 1) {
        fprintf(stderr, "Error reading max steps\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Čítanie pravdepodobností pohybu
    int probabilities[4];
    if (fscanf(file, "%d, %d, %d, %d\n", &probabilities[0], &probabilities[1], &probabilities[2], &probabilities[3]) != 4) {
        fprintf(stderr, "Error reading movement probabilities\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Nastavenie pravdepodobností (z percent na desatinné čísla)
    for (int i = 0; i < 4; i++) {
        state->probabilities[i] = probabilities[i];
    }

    // Čítanie modu
    int mode = 0;
    if (fscanf(file, "%d\n", mode) != 1) {
        fprintf(stderr, "Error reading max steps\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    if (mode == 1) {
        state->interactive_mode = true;
        state->summary_mode = false;
    } else {
        state->interactive_mode = false;
        state->summary_mode = true;
    }

    // Alokácia a čítanie sveta toto asi netreba lebo sa svet vytvori novy pri kazdej replikacii.
    // state->world = malloc(state->world_width_ * sizeof(char **));
    // for (int i = 0; i < state->world_width_; i++) {
    //     state->world[i] = malloc(state->world_height_ * sizeof(char *));
    //     for (int j = 0; j < state->world_height_; j++) {
    //         state->world[i][j] = malloc(5 * sizeof(char)); // 4 znaky + '\0'
    //         if (fscanf(file, "%4s", state->world[i][j]) != 1) {
    //             fprintf(stderr, "Error reading world data at [%d][%d]\n", i, j);
    //             fclose(file);
    //             exit(EXIT_FAILURE);
    //         }
    //     }
    // }

    fclose(file);
}


int main(void) {
    srand(time(NULL));
    WalkerState walker;
    SimulationState state;
    init_walker(&walker);
    init_simulation(&state, &walker, false, true, false);
    //single_walk(&state, 0, 0);
    if (state.interactive_mode) {
        simulate_walk(&state);
    }
    else {
        calculate_probability_to_reach_center(&state);
    }
    write_to_file("../worlds_load/output.txt", &state);
    destroy_simulation(&state);
    return 0;
}
