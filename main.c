#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include "initData.h"

int main(int argc, char *argv[]) {
    int world_width, world_height, max_steps, num_replicas, obstaclesAllowed;
    double probs[4];

    // Získanie rozmerov sveta
    printf("Zadajte šírku sveta: ");
    scanf("%d", &world_width);
    printf("Zadajte výšku sveta: ");
    scanf("%d", &world_height);

    // Získanie počtu replikácií
    printf("Zadajte počet replikácií: ");
    scanf("%d", &num_replicas);

    printf("Zadajte pravdepodobnosťi pohybu chodcu [hore, dole, vpravo, vlavo]: v desatinych cislach npr: [0.5, 0.2, 0.1, 0.2]");
    scanf("%f %f %f %f", probs[0], probs[1], probs[2], probs[3]);

    // Overenie, že pravdepodobnosti sú validné
    float total_prob = probs[0] + probs[1] + probs[2] + probs[3];
    if (total_prob >= 1.0) {
        printf("Súčet pravdepodobností musí byť menší ako 1.0! (Zadali ste: %.2f)\n", total_prob);
        return 1;
    }

    // Získanie maximálneho počtu krokov
    printf("Zadajte maximálny počet krokov: ");
    scanf("%d", &max_steps);

    printf("Svet s prekazkami - 1 | Svet bez prekazok - 2: ");
    scanf("%d", &obstaclesAllowed);

    if (obstaclesAllowed != 1 || obstaclesAllowed != 0) {
        printf("Zla zadana hodnota pre prekazky (Zadali ste: %d)\n", obstaclesAllowed);
        return 1;
    }

    // Vytvorenie FIFO (ak už neexistuje)
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        perror("mkfifo");
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

    // Zatvorenie FIFO
    close(fifo_fd);
    return 0;
}