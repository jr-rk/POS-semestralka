// client.h
#ifndef CLIENT
#define CLIENT

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
#include "shared_memory.h"


void print_world_client(sharedData *state);

void print_usage(const char *prog_name);

int main(int argc, char *argv[]);


#endif // !CLIENT
