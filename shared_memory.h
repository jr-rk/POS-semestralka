//
// Created by maros on 10. 1. 2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define MAX_WIDTH 20
#define MAX_HEIGHT 20

#define SHM_NAME "shm_memory"
#define SEM_READ "/sem_read"
#define SEM_WRITE "/sem_write"

typedef struct sharedData {
    char grid[MAX_WIDTH][MAX_HEIGHT][5];           // Hlavná mriežka
    char grid_prob[MAX_WIDTH][MAX_HEIGHT][5];      // Pravdepodobnosti
    int obstaclesAllowed;                          // Povolené prekážky
    int walker_x;                                  // Počiatočná pozícia chodca
    int walker_y;                                  // Počiatočná pozícia chodca
    int current_rep;
    int width;
    int height;
    int interactive_mode;
} sharedData;

#endif //SHARED_MEMORY_H
