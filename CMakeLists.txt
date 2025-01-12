cmake_minimum_required(VERSION 3.10)

# Názov projektu
project(NahodnaPochodzka3 C)

# Štandard C (voliteľné, ak potrebujete špecifický štandard)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Spustiteľný súbor pre main.c
add_executable(client client.c)

# Spustiteľný súbor pre server.c
add_executable(server server.c)

# Zahrnutie hlavičkových súborov
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})

# Linkovanie potrebných knižníc
target_link_libraries(client PRIVATE pthread rt)
target_link_libraries(server PRIVATE pthread rt)
