cmake_minimum_required(VERSION 3.8)

project(demo3d VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -O3")

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED) 
find_package(glm REQUIRED)

include_directories(
    ${PROJECT_NAME} PUBLIC
    ${SDL2_INCLUDE_DIRS} 
    "${PROJECT_BINARY_DIR}" 
    "${PROJECT_SOURCE_DIR}/include"
)

add_executable(
    demo3d
    src/main.cpp
)

target_link_libraries(demo3d ${SDL2_LIBRARIES} m glm SDL2_image SDL2_ttf)
