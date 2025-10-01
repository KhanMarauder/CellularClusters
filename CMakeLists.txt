cmake_minimum_required(VERSION 3.15)
project(CellularClusters)

find_package(OpenCL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(main src/main.cpp)

# Include SDL2 and SDL2_image headers
target_include_directories(main PRIVATE
	${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)

# Link OpenCL, SDL2, and SDL2_image
target_link_libraries(main
	OpenCL::OpenCL
	${SDL2_LIBRARIES}
	${SDL2_IMAGE_LIBRARIES}
)
