#include "client.h"


void print_world_client(SharedData *state) {
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
void print_usage(const char *prog_name) {
  printf("Usage: %s <option>\n", prog_name);
  printf("Options:\n");
  printf("  1  Create a new simulation\n");
  printf("  2  Connect to an existing simulation\n");
  printf("  3  Load simulation from a file\n");
}

int main(int argc, char *argv[]) {

  if (argc > 3) {
    fprintf(stderr, "Error: too many arguments.\n");
    print_usage(argv[0]);
    return 1;
  }
  int option = atoi(argv[1]);

  int world_width, world_height, max_steps, num_replicas, obstaclesAllowed, mode;
  double probs[4];

  switch (option) {
    case 1:


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

      InitData init_Data;
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
      if (write(fifo_fd, &init_Data, sizeof(InitData)) != sizeof(InitData)) {
        perror("write");
        exit(1);
      }

      printf("Data sent to server successfully.\n");

      sleep(3);
      // Zatvorenie FIFO
      close(fifo_fd);
      break;
    case 2:
      printf("Pripojenie k existujuce pamati: %s \n", SHM_NAME);
      break;
    case 3:
      printf("Nacitanie zo suboru: \n");
      FILE *file = fopen(argv[2], "r");
      if (file == NULL) {
        perror("Error opening file");
        return 1;
      }

      fscanf(file, "%d %d %d", &world_width, &world_height, &num_replicas);
      for (int i = 0; i < 4; i++) {
        fscanf(file, "%lf", &probs[i]);
      }
      fscanf(file, "%d %d %d", &max_steps, &obstaclesAllowed, &mode);

      fclose(file);
      // Vytvorenie FIFO (ak už neexistuje)
      if (mkfifo(FIFO_NAME, 0666) == -1) {
        //unlink(FIFO_NAME);
        perror("mkfifo");
      }

      if (mode != 1 && mode != 2) {
        printf("Zla zadana hodnota pre mode (Zadali ste: %d)\n", mode);
        return 1;
      }

      InitData init_Data1;
      init_Data1.width = world_width;
      init_Data1.height = world_height;
      init_Data1.max_steps = max_steps;
      init_Data1.probabilities[0] = probs[0];
      init_Data1.probabilities[1] = probs[1];
      init_Data1.probabilities[2] = probs[2];
      init_Data1.probabilities[3] = probs[3];
      init_Data1.obstaclesAllowed = obstaclesAllowed;
      init_Data1.mode = mode;
      init_Data1.num_reps = num_replicas;

      int fifo_fd1 = open(FIFO_NAME, O_WRONLY);
      if (fifo_fd1 == -1) {
        perror("open");
        exit(1);
      }

      // Zápis údajov do FIFO
      if (write(fifo_fd1, &init_Data1, sizeof(InitData)) != sizeof(InitData)) {
        perror("write");
        exit(1);
      }

      printf("Data sent to server successfully.\n");

      sleep(3);
      // Zatvorenie FIFO
      close(fifo_fd1);
      break;
  }



  // Otváranie semaforov pre synchronizáciu
  sem_t *sem_write = sem_open(SEM_WRITE, 0);  // Semafor pre zápis
  sem_t *sem_read = sem_open(SEM_READ, 0);    // Semafor pre čítanie
  sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);

  if (sem_write == SEM_FAILED) {
    perror("Semafor otvorenie zlyhalo 3 write");
    exit(1);
  }
  if (sem_read == SEM_FAILED) {
    perror("Semafor otvorenie zlyhalo 3 read");
  }
  if (sem_mutex == SEM_FAILED) {
    perror("Semafor mutex v clientovy otvorenie zlyhalo 3 read");
  }

  // Pripojenie k zdieľanej pamäti
  int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed 3");
    exit(1);
  }

  // Pripojenie zdieľanej pamäti
  SharedData *shm_data = (SharedData *)mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_data == MAP_FAILED) {
    perror("mmap failed 3");
    exit(1);
  }

  if (shm_data->interactive_mode) {
    while (1) {
      sem_wait(sem_read);

      if (shm_data->server_active == 0) {
        break;
      }

      sem_wait(sem_mutex);
      shm_data->readers_active++;
      //printf("Zvysil som pocet citatelov: %d \n", shm_data->readers_active);
      sem_post(sem_mutex);

      if (shm_data->done) {

        printf("\n Chodec zacal na suradniciach: [%d, %d] \n", shm_data->walker_y, shm_data->walker_x);
        printf("Replikacia c. : %d \n", shm_data->current_rep);
        print_world_client(shm_data);
        sem_wait(sem_mutex);
        shm_data->readers_active--;
        sem_post(sem_mutex);

        sem_post(sem_write);
        break;
      }

      printf("Chodec zacal na suradniciach: [%d, %d] \n", shm_data->walker_y, shm_data->walker_x);
      printf("Replikacia c. : %d \n", shm_data->current_rep);
      print_world_client(shm_data);

      sem_wait(sem_mutex);
      shm_data->readers_active--;
      //printf("Znizil som pocet citatelov: %d \n", shm_data->readers_active);
      sem_post(sem_mutex);
      sem_post(sem_write);

      sleep(1);
    }
  } else {
    while (1) {
      sem_wait(sem_read);
      if (!shm_data->server_active) {
        break;
      }
      sem_wait(sem_mutex);
      shm_data->readers_active++;
      sem_post(sem_mutex);

      if (shm_data->done) {
        printf("\n Chodec zacal na suradniciach: [%d, %d] \n", shm_data->walker_y, shm_data->walker_x);
        print_world_client(shm_data);

        sem_wait(sem_mutex);
        shm_data->readers_active--;
        sem_post(sem_mutex);

        sem_post(sem_write);
        break;
      }
      printf("Chodec zacal na suradniciach: [%d, %d] \n", shm_data->walker_y, shm_data->walker_x);
      print_world_client(shm_data);

      sem_wait(sem_mutex);
      shm_data->readers_active--;
      sem_post(sem_mutex);

      sem_post(sem_write);

      sleep(1);
    }
  }
  //odpojenie a odstranenie
  munmap(shm_data, sizeof(SharedData));
  close(shm_fd);
  sem_close(sem_write);
  sem_close(sem_read);
  sem_close(sem_mutex);

  return 0;
}
