//
// Created by maros on 8. 1. 2025.
//

#ifndef INITDATA_H
#define INITDATA_H
typedef struct initData {
    int width;
    int height;
    int num_reps;
    double probabilities[4];
    int max_steps;
    int obstaclesAllowed; // 1 - obstacles 0 - without
} initData;

#define FIFO_NAME "fifo_pipe"
#endif //INITDATA_H
