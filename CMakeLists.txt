cmake_minimum_required(VERSION 3.13)
project(IT8951 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Test
        interface.c
        interface.h
        test.c
        )

add_library(IT8951 SHARED
        interface.c
        interface.h
        )

find_library(BCM2835 bcm2835)
target_link_libraries(Test ${BCM2835})
target_link_libraries(IT8951 ${BCM2835})