#ifndef SHARED_DATA_H
#define SHARED_DATA_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#define MAX_WIDTH 20
#define MAX_HEIGHT 20

#define SHM_NAME "shm_memory__0007"
#define SEM_READ "/sem_read__0007"
#define SEM_WRITE "/sem_write__0007"
#define SEM_MUTEX "/sem_mutex__0007"
#define WRITER_EXEC "./server"
#define READER_EXEC "./client"


typedef struct SharedData {
  char grid[MAX_WIDTH][MAX_HEIGHT][5];           // Hlavná mriežka
  char grid_prob[MAX_WIDTH][MAX_HEIGHT][5];      // Pravdepodobnosti
  int obstaclesAllowed;                          // Povolené prekážky
  int walker_x;                                  // Počiatočná pozícia chodca
  int walker_y;                                  // Počiatočná pozícia chodca
  int current_rep;
  int width;
  int height;
  int interactive_mode;
  int obstacles_count;
  int done;
  int readers_active;
  int server_active;
} SharedData;

#endif //SHARED_DATA_H
