cmake_minimum_required(VERSION 3.30)
project(drunkardsWalk2 C)

set(CMAKE_C_STANDARD 11)

add_executable(drunkardsWalk2 main.c
        server.c
        server.h
        world.c
        world.h
        initData.h)
