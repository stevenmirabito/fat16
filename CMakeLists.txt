cmake_minimum_required(VERSION 3.13)
project(filesystem C)

set(CMAKE_C_STANDARD 99)

add_executable(fstest fstest.c fat16.h mbr.h driver.h fat16.c fs_drivers/virtual.c fs_drivers/virtual.h fs_drivers/testimg.h)
