#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "initData.h"
#include "shared_memory.h"

void print_world_client(sharedData *state) {
    printf("     ");
    for (int j = 0; j < state->height; j++) {
        if (j < 10) {
            printf(" 0%d  ", j);
        } else {
            printf(" %d  ", j);
        }
    }
    printf("\n");
    for (int i = 0; i < state->width; i++) {
        printf("%3d |", i);
        for (int j = 0; j < state->height; j++) {
            printf("%s ", state->grid[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int world_width, world_height, max_steps, num_replicas, obstaclesAllowed, mode;
    double probs[4];

    // Získanie rozmerov sveta
    printf("Zadajte šírku sveta: ");
    scanf("%d", &world_width);
    printf("Zadajte výšku sveta: ");
    scanf("%d", &world_height);

    // Získanie počtu replikácií
    printf("Zadajte počet replikácií: ");
    scanf("%d", &num_replicas);

    printf("Zadajte pravdepodobnosťi pohybu chodcu [hore, dole, vpravo, vlavo]: v desatinych cislach npr: [0.5, 0.2, 0.1, 0.2] ");
    scanf("%lf %lf %lf %lf", &probs[0], &probs[1], &probs[2], &probs[3]);

    // Overenie, že pravdepodobnosti sú validné
    double total_prob = probs[0] + probs[1] + probs[2] + probs[3];
    if (total_prob > 1.0) {
        printf("Súčet pravdepodobností musí byť menší ako 1.0! (Zadali ste: %.2f)\n", total_prob);
        return 1;
    }

    // Získanie maximálneho počtu krokov
    printf("Zadajte maximálny počet krokov: ");
    scanf("%d", &max_steps);

    printf("Svet s prekazkami - 1 | Svet bez prekazok - 0: ");
    scanf(" %d", &obstaclesAllowed);

    if (obstaclesAllowed != 1 && obstaclesAllowed != 0) {
        printf("Zla zadana hodnota pre prekazky (Zadali ste: %d)\n", obstaclesAllowed);
        return 1;
    }

    printf("Mod interaktivny - 1  | Sumarny - 2: ");
    scanf("%d", &mode);

    // Vytvorenie FIFO (ak už neexistuje)
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        //unlink(FIFO_NAME);
        perror("mkfifo");
    }

    if (mode != 1 && mode != 2) {
        printf("Zla zadana hodnota pre mode (Zadali ste: %d)\n", mode);
        return 1;
    }

    initData init_Data;
    init_Data.width = world_width;
    init_Data.height = world_height;
    init_Data.max_steps = max_steps;
    init_Data.probabilities[0] = probs[0];
    init_Data.probabilities[1] = probs[1];
    init_Data.probabilities[2] = probs[2];
    init_Data.probabilities[3] = probs[3];
    init_Data.obstaclesAllowed = obstaclesAllowed;
    init_Data.mode = mode;
    init_Data.num_reps = num_replicas;
    int fifo_fd = open(FIFO_NAME, O_WRONLY);
    if (fifo_fd == -1) {
        perror("open");
        exit(1);
    }

    // Zápis údajov do FIFO
    if (write(fifo_fd, &init_Data, sizeof(initData)) != sizeof(initData)) {
        perror("write");
        exit(1);
    }

    printf("Data sent to server successfully.\n");

    // Otváranie semaforov pre synchronizáciu
    sem_t *sem_write = sem_open(SEM_WRITE, 0);  // Semafor pre zápis
    sem_t *sem_read = sem_open(SEM_READ, 0);    // Semafor pre čítanie

    if (sem_write == SEM_FAILED || sem_read == SEM_FAILED) {
        perror("Semafor otvorenie zlyhalo 3");
        exit(1);
    }

    // Pripojenie k zdieľanej pamäti
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed 3");
        exit(1);
    }

    // Pripojenie zdieľanej pamäti
    sharedData *shm_data = (sharedData *)mmap(NULL, sizeof(sharedData), PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_data == MAP_FAILED) {
        perror("mmap failed 3");
        exit(1);
    }

    if (shm_data->interactive_mode == 1) {
        for (int x = 0; x < shm_data->width; x++) {
            for (int y = 0; y < shm_data->height; y++) {
                for (int t = 0; t < num_replicas; t++) {
                    sem_wait(sem_read); //server moze spracovat data
                    //vykreslenie data
                    printf("Chodec zacal na suradniciach: [%d, %d] \n", shm_data->walker_x, shm_data->walker_y);
                    printf("Replikacia c. : %d ", shm_data->current_rep);
                    print_world_client(shm_data);
                    sem_post(sem_write);

                }
            }
        }
    } else {
        for (int x = 0; x < shm_data->width; x++) {
            for (int y = 0; y < shm_data->height; y++) {
                sem_wait(sem_read); //server moze spracovat data
                //vykreslenie data
                printf("Pravdepodobnost ze chodec dojde do ciela z suradnic: [%d, %d] \n", shm_data->walker_x, shm_data->walker_y);
                print_world_client(shm_data);
                sem_post(sem_write);

            }
        }
    }

    //odpojenie a odstranenie
    munmap(shm_data, sizeof(sharedData));
    close(shm_fd);
    sem_close(sem_write);
    sem_close(sem_read);

    // Zatvorenie FIFO
    close(fifo_fd);

    return 0;
}
