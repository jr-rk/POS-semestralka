#ifndef INITDATA_H
#define INITDATA_H


#define FIFO_NAME "fifo_pipe_3210"

typedef struct InitData {
  int width;
  int height;
  int num_reps;
  double probabilities[4];
  int max_steps;
  int obstaclesAllowed; // 1 - obstacles 0 - without
  int mode; // 1 interactive | 2 - summary
} InitData;

#endif //INITDATA_H
