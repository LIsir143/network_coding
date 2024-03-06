cmake_minimum_required(VERSION 3.0)

project(CLIENT)

include_directories(include)

add_executable(client_cmake main.cpp src/client.cpp)