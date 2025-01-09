//
// Created by maros on 8. 1. 2025.
//
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>
#include "world.h"
#include "initData.h"


int main(int argc, char *argv[]) {
    // Vytvorenie FIFO
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }

    printf("Waiting for client...\n");

    // Otvorenie FIFO na čítanie
    int fifo_fd = open(FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1) {
        perror("open");
        exit(1);
    }

    // Čítanie údajov zo štruktúry
    initData data;
    if (read(fifo_fd, &data, sizeof(initData)) != sizeof(initData)) {
        perror("read");
        exit(1);
    }

    //spracovanie dat
    WalkerState walker;
    SimulationState state;
    init_walker(&walker);
    init_simulation(&state, &walker, &data);

    printf("Initialized data are: width = %d, height = %d, num_reps = %d, max_steps = %d, "
           "obstacles = %d\n", state.world_width_, state.world_height_, state.num_replications,
           state.max_steps_, state.obstacles);

    close(fifo_fd);
    unlink(FIFO_NAME);
    destroy_simulation(&state);

    return 0;
}
