cmake_minimum_required(VERSION 3.15)
project(CellularClusters)

find_package(OpenCL REQUIRED)
find_package(SDL2 REQUIRED)
add_executable(main src/main.cpp)

# Include SDL2 headers
target_include_directories(main PRIVATE
		${SDL2_INCLUDE_DIRS}
)

# Link OpenCL and SDL2
target_link_libraries(main
		OpenCL::OpenCL
		${SDL2_LIBRARIES}
		SDL2_image
)
