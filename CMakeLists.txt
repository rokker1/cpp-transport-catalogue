cmake_minimum_required(VERSION 3.10)

# set the project name
project(Transport-catalogue)

# add the executable
add_executable(Transport-catalogue 
    transport-catalogue/main.cpp
    transport-catalogue/input_reader.cpp
    transport-catalogue/stat_reader.cpp
    transport-catalogue/transport_catalogue.cpp
    transport-catalogue/input_reader.h
    transport-catalogue/stat_reader.h
    transport-catalogue/transport_catalogue.h)