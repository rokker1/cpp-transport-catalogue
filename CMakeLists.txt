cmake_minimum_required(VERSION 3.10)

# set the project name
project(Transport-catalogue)

# add the executable
add_executable(Transport-catalogue 
    transport-catalogue/main.cpp
    transport-catalogue/json_reader.cpp
    transport-catalogue/domain.cpp
    transport-catalogue/transport_catalogue.cpp
    transport-catalogue/map_renderer.cpp
    transport-catalogue/request_handler.cpp
    transport-catalogue/svg.cpp
    transport-catalogue/json.cpp
    transport-catalogue/geo.cpp)