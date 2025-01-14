#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "initData.h"
#include "sharedData.h"


void print_world_client(SharedData *state);

void print_usage(const char *prog_name);

#endif // !CLIENT_H
