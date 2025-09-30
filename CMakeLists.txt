cmake_minimum_required(VERSION 3.15)
project(CellularClusters)

find_package(OpenCL REQUIRED)

add_executable(main src/main.cpp)
target_link_libraries(main OpenCL::OpenCL)
