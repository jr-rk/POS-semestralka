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
#include "shared_memory.h"


//zatial nastavim tieto hodnoty rucne
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

// char* pridaj_kluc_na_koniec(char* input, int kluc) {
//     int input_len = strlen(input);
//     char* result = (char*)malloc(input_len + 4); //
//
//     if (result == NULL) {
//         printf("Chyba pri alokácii pamäte.\n");
//         exit(1);
//     }
//     strcpy(result, input);
//     snprintf(result + input_len, 4, "%03d", kluc);
//     return result;
// }

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
    if (state->world == NULL) {
        fprintf(stderr, "Allocation failed for world\n");
        exit(1);
    }
    state->prob_world = (double**)malloc(data->width * sizeof(double *));
    if (state->prob_world == NULL) {
        fprintf(stderr, "Allocation failed for prob_world\n");
        exit(1);
    }
    state->world_width_ = data->width;
    state->world_height_ = data->height;
    state->max_steps_ = data->max_steps;
    state->probabilities[0] = data->probabilities[0];
    state->probabilities[1] = data->probabilities[1];
    state->probabilities[2] = data->probabilities[2];
    state->probabilities[3] = data->probabilities[3];
    state->num_replications = data->num_reps;
    state->interactive_mode = (data->mode == 1 ? true : false);
    state->summary_mode = (data->mode == 2 ? true : false);
    state->obstaclesAllowed = (data->obstaclesAllowed == 1 ? true : false); // ak si uzivatel vybere svet s prekazkami tak TRUE
    state->obstacleCount = (int)(data->width * data->height) * (0.2);
    state->obstacles = malloc(state->obstacleCount * sizeof(Object));
    if (state->obstacles == NULL) {
        fprintf(stderr, "Allocation failed for obstacles\n");
        exit(1);
    }
    state->walker = walker;

    for (int i = 0; i < state->max_steps_; i++) {
        state->walker->trajectory[i].x = -1;
        state->walker->trajectory[i].y = -1;
        state->walker->trajectory[i].direction = -1;
    }

    //init world interaktivny rezim
    for (int i = 0; i < state->world_width_; i++) {
        state->world[i] = malloc(state->world_height_ * sizeof(char *));
        if (state->world[i] == NULL) {
            fprintf(stderr, "Allocation failed for world row %d\n", i);
            exit(1);
        }
        for (int j = 0; j < state->world_height_; j++) {
            state->world[i][j] = malloc(10 * sizeof(char)); // Pre ulozenie textu
            if (state->world[i][j] == NULL) {
                fprintf(stderr, "Allocation failed for world[%d][%d]\n", i, j);
                exit(1);
            }
            snprintf(state->world[i][j], 10, SPACE);
        }
    }

    //init world na pravdepodobnosti
    for (int i = 0; i < state->world_width_; i++) {
        state->prob_world[i] = malloc(state->world_height_ * sizeof(double)); //alokacia kazdeho riadku
        if (state->prob_world[i] == NULL) {
            fprintf(stderr, "Allocation failed for prob_world row %d\n", i);
            exit(1);
        }
        for (int j = 0; j < state->world_height_; j++) {
            state->prob_world[i][j] = 0.0;
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

    //printf("START WALKING, Walker on position [%d, %d] \n", walker->x, walker->y);
    update_world(state);
    //print_world(state);

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
                    //printf("KROK: %d - Chodec sa pokusil stupit na prekazku[%d, %d] - nemozes \n", step, new_x, new_y);
                } //- toto asi tiez mozes vymazat
            } else {
                walker->x = new_x;
                walker->y = new_y;
            }
            walker->step++;

            if (walker->x == center_x && walker->y == center_y) {
                //printf("--------------------------------------\n");
                //printf("Step %d: WALKER[%d, %d] DOSIAHOL STRED V %d KROKOCH\n",step+1, walker->x, walker->y, step+1);
                if (state->interactive_mode) {
                    update_world(state);
                }
                //print_world(state);
                walker->centerReached = true;
                break;
            }
        }

        if (state->interactive_mode) {
            //printf("--------------------------------------\n");
            //printf("Step %d: Walker is on position [%d, %d] ->", walker->step, walker->x, walker->y);
            //printf(" Stred sveta: [%d, %d] \n ", center_x, center_y);
            update_world(state);
            //print_world(state);
        }

        if (state->summary_mode) {
            //printf("--------------------------------------\n");
            //printf("Step %d: Walker is on position [%d, %d] -> ", walker->step, walker->x, walker->y);
            //printf(" Stred sveta: [%d, %d] \n ", center_x, center_y);
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

void shared_memory_write(SimulationState *state, sharedData *data, int walker_x, int walker_y, int current_rep, int done) {
    //update grid_interaktive|summary, obstaclesAllowed, x, y walker, current_rep
    if (state->interactive_mode) {
        for (int x = 0; x < state->world_width_; x++) {
            for (int y = 0; y < state->world_height_; y++) {
                snprintf(data->grid[x][y], 5, state->world[x][y]);
            }
        }
    } else {
        for (int x = 0; x < state->world_width_; x++) {
            for (int y = 0; y < state->world_height_; y++) {
                snprintf(data->grid[x][y], 5, state->world[x][y]);
            }
        }
    }


    data->walker_x = walker_x;
    data->walker_y = walker_y;
    data->obstaclesAllowed = state->obstaclesAllowed == true ? 1 : 0;
    data->current_rep = current_rep;
    data->width = state->world_width_;
    data->height = state->world_height_;
    data->interactive_mode = state->interactive_mode == true ? 1 : 0;
    data->obstacles_count = state->obstacleCount;
    data->done = done;
}

void simulate_walk(SimulationState *state, sharedData *shm_data) {
    //otvaranie semaforov
    sem_t *sem_write = sem_open(SEM_WRITE, O_CREAT, 0644, 1); // Semafor pre zápis
    sem_t *sem_read = sem_open(SEM_READ, O_CREAT, 0644, 0);  // Semafor pre čítanie
    sem_t *sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0644, 1); // na ochranu citatelov

    if (sem_write == SEM_FAILED || sem_read == SEM_FAILED || sem_mutex == SEM_FAILED) {
        perror("Semafor otvorenie zlyhalo 1");
        exit(1);
    }
    int center_x = state->world_width_ / 2;
    int center_y = state->world_height_ / 2;

    for (int x = 0; x < state->world_width_; x++) {
        for (int y = 0; y < state->world_height_; y++) {
            if (!is_obstacle(state, x, y) && !(x == center_x && y == center_y)) {
                for (int t = 0; t < state->num_replications; t++) {

                    //cakaj kym clienti nedokoncia vypisovanie
                    sem_wait(sem_mutex);
                    while (shm_data->readers_active > 0) {
                        sem_post(sem_mutex);  // Uvoľni mutex pre čitateľov
                        usleep(1000);
                        sem_wait(sem_mutex);  // Získaj mutex pred ďalšou kontrolou
                        printf("Cakam, kym vsetci citatelia nebudu hotovi: %d \n ", shm_data->readers_active);
                    }

                    sem_post(sem_mutex); //uvolni mutex po skonceni cakania
                    //printf("Writer: All readers are done, proceeding...\n");

                    sem_wait(sem_write);
                    //printf("******************************************************* \n ");
                    //printf("ZACIATOK REPLIKACIE %d \n ", t + 1);
                    single_walk(state, x, y);
                    //print_world(state);
                    if (x == state->world_width_ - 1 && y == state->world_height_ - 1  && t == state->num_replications - 1) {
                        shared_memory_write(state, shm_data, y, x, t, 1);
                    } else {
                        shared_memory_write(state, shm_data, y, x, t, 0);
                    }

                    sem_post(sem_read);
                    

                    //sleep(1);
                }
            }
            else {
                //printf("Preskakuje sa bod [%d][%d], pretoze je to prekazka.\n", x, y);
            }
        }
    }
    //ukoncenie servera
    sem_wait(sem_write);
    shm_data->server_active = 0;
    sem_post(sem_read);

    sem_close(sem_write);
    sem_close(sem_read);
    sem_close(sem_mutex);
    sem_unlink(SEM_WRITE);
    sem_unlink(SEM_READ);
    sem_unlink(SEM_MUTEX);
}



void calculate_probability_to_reach_center(SimulationState *state, sharedData *shm_data) {
    //otvaranie semaforov
    sem_t *sem_write = sem_open(SEM_WRITE, O_CREAT, 0644, 1); // Semafor pre zápis
    sem_t *sem_read = sem_open(SEM_READ, O_CREAT, 0644, 0);  // Semafor pre čítanie
    sem_t *sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0644, 1); // na ochranu citatelov

    if (sem_write == SEM_FAILED || sem_read == SEM_FAILED || sem_mutex == SEM_FAILED) {
        perror("Semafor otvorenie zlyhalo 2");
        exit(1);
    }
    WalkerState *walker = state->walker;
    int center_x = state->world_width_ / 2;
    int center_y = state->world_height_ / 2;
    for (int x = 0; x < state->world_width_; x++) {
        for (int y = 0; y < state->world_height_; y++) {

            //aby nerobil replikacie tam kde su prekazky a stred
             if (!is_obstacle(state, x, y) && !(x == center_x && y == center_y)) {

                 //cakaj kym clienti nedokoncia vypisovanie
                 sem_wait(sem_mutex);
                 while (shm_data->readers_active > 0) {
                     sem_post(sem_mutex);  // Uvoľni mutex pre čitateľov
                     usleep(1000);
                     sem_wait(sem_mutex);  // Získaj mutex pred ďalšou kontrolou
                 }

                 sem_post(sem_mutex); //uvolni mutex po skonceni cakania
                 //printf("Writer: All readers are done, proceeding...\n");

                 sem_wait(sem_write);

                int succesful_reaches_center = 0;

                for (int t = 0; t < state->num_replications; t++) {
                    //printf("******************************************************* \n ");
                    //printf("ZACIATOK REPLIKACIE %d \n ", t + 1);
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
                 //print_world(state);
                 if (x == state->world_width_ - 1 && y == state->world_height_ - 1) {
                     shared_memory_write(state, shm_data, y, x, 0, 1);
                 } else {
                     shared_memory_write(state, shm_data, y, x, 0, 0);
                 }
                 sem_post(sem_read);


                 //sleep(1);
            } else {
                //printf("Preskakuje sa bod [%d][%d], pretoze je to prekazka.\n", x, y);
            }

        }
    }
    //ukoncenie servera
    sem_wait(sem_write);
    shm_data->server_active = 0;
    sem_post(sem_read);

    sem_close(sem_write);
    sem_close(sem_read);
    sem_close(sem_mutex);
    sem_unlink(SEM_WRITE);
    sem_unlink(SEM_READ);
    sem_unlink(SEM_MUTEX);
}

void zapis_do_suboru(SimulationState *state) {
    FILE *outfile = fopen("output_config.txt", "w");
    if (outfile == NULL) {
        perror("Error opening output file");
    }

    fprintf(outfile, "%d %d %d\n", state->world_width_, state->world_height_, state->num_replications);

    fprintf(outfile, "%.2f %.2f %.2f %.2f \n", state->probabilities[0], state->probabilities[1], state->probabilities[2], state->probabilities[3]);

    fprintf(outfile, "%d %d %d\n", state->max_steps_, state->obstaclesAllowed, state->interactive_mode == 1 ? 1 : 2);

    fprintf(outfile, "     ");
    for (int j = 0; j < state->world_height_; j++) {
        if (j < 10) {
            fprintf(outfile," 0%d  ", j);
        } else {
            fprintf(outfile, " %d  ", j);
        }
    }
    fprintf(outfile, "\n");
    for (int i = 0; i < state->world_width_; i++) {
        fprintf(outfile,"%3d |", i);
        for (int j = 0; j < state->world_height_; j++) {
            fprintf(outfile, "%s ", state->world[i][j]);
        }
        fprintf(outfile, "\n");
    }
    fclose(outfile);
    printf("Data boli zapisane do output_config.txt\n");
}

void print_usage(const char *prog_name) {
    printf("Pouzitie: %s <moznost>\n", prog_name);
    printf("Moznosti:\n");
    printf("  1  Vytvorenie novej simulacie\n");
    printf("  2  Pripojit sa k existujucej\n");
    printf("  3 <cesta k suboru> Nacitat simulaciu zo suboru\n");
}

void create_new_simulation(char *argv[]) {

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        //sme v procese child
        printf("Spustam klienta\n");
        execv(READER_EXEC, argv);
        perror("execl failed");
        exit(1);
    } else {
        // sme v procese parent
        // Vytvorenie FIFO
        if (mkfifo(FIFO_NAME, 0666) == -1) {
            perror("mkfifo");
            exit(1);
        }

        printf("Cakanie na klienta...\n");


        // Otvorenie FIFO na čítanie
        int fifo_fd = open(FIFO_NAME, O_RDONLY);
        if (fifo_fd == -1) {
            perror("open");
            exit(1);
        }

        // Čítanie údajov zo štruktúry - fifo
        initData data_fifo;
        if (read(fifo_fd, &data_fifo, sizeof(initData)) != sizeof(initData)) {
            perror("read");
            exit(1);
        }


        // Vytvorenie alebo pripojenie k zdieľanej pamäti
        int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            perror("shm_open failed 1");
            exit(1);
        }

        // Nastavenie veľkosti zdieľanej pamäti
        if (ftruncate(shm_fd, sizeof(sharedData)) == -1) {
            perror("ftruncate failed 1");
            exit(1);
        }

        // Pripojenie zdieľanej pamäti
        sharedData *shm_data = (sharedData *)mmap(NULL, sizeof(sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shm_data == MAP_FAILED) {
            perror("mmap failed 1");
            exit(1);
        }

        //spracovanie dat fifo
        WalkerState walker;
        SimulationState state;
        init_walker(&walker);

        init_simulation(&state, &walker, &data_fifo);
        shared_memory_write(&state, shm_data, 0, 0, 0, 0);
        shm_data->readers_active = 0;
        shm_data->server_active = 1;

        //printf("Init Data: prob1 %lf, readers %d, steps %d\n", state.probabilities[0],shm_data->readers_active , state.max_steps_);


        //run_simulation(&state);
        if (state.interactive_mode) {
            simulate_walk(&state, shm_data);
        }
        else {
            calculate_probability_to_reach_center(&state, shm_data);
        }
        zapis_do_suboru(&state);

        destroy_simulation(&state);

        close(fifo_fd);
        unlink(FIFO_NAME);
        munmap(shm_data, sizeof(sharedData));
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }
}

int connect_to_existing_simulation( char *argv[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        //sme v procese child
        printf("Spustam klienta\n");
        execv(READER_EXEC, argv);
        perror("execl failed");
        exit(1);
    } else {
        return 0;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    if (argc > 3) {
        fprintf(stderr, "Error: Too many arguments\n");
        print_usage(argv[0]);
        return 1;
    }
    int option = atoi(argv[1]);

    switch (option) {
        case 1:
            printf("Vytvaram novu simulaciu \n");
        create_new_simulation(argv);
        break;
        case 2:
            printf("Pripajam sa k existujucej \n");
        return connect_to_existing_simulation(argv);
        case 3:
            printf("Nacitam simulaciu zo suboru \n");
            create_new_simulation(argv);
        break;
    }


    return 0;
}
