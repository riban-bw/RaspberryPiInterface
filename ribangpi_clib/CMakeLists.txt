cmake_minimum_required(VERSION 3.0)
project(riban-rpi-gpi)

include(CheckIncludeFiles)
include(CheckLibraryExists)

link_directories(/usr/local/lib)

message("Building riban RPi GPI library")
add_library(ribangpi SHARED i2c.c i2c.h gpi.c gpi.h rpigpi.h rpigpi.c mcp23017gpi.c mcp23017gpi.h)
target_link_libraries(ribangpi)
