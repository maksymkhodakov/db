cmake_minimum_required(VERSION 3.22)
project(Lab_1_2 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Lab_1_2
        Checks.h
        Input.h
        Main.c
        Train.h
        Output.h
        Crew.h
        Entities.h)
