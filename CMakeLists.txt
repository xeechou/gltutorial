cmake_minimum_required(VERSION 3.1.0)

project(mysamples
  VERSION 0.1
  LANGUAGES C CXX)

set(CMAKE_C_FLAGS "-g")
add_definitions(
  -D_GNU_SOURCE
  )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -g")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/CMake")
#set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build"), you cant have it

find_package(PkgConfig REQUIRED)

find_package(GLEW  REQUIRED)
find_package(glm   REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(assimp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
#find_package(GLFW REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(include)
add_subdirectory(common)

#tutorial folders
#add_subdirectory(t0_triangle)
#add_subdirectory(t2_matrix)
add_subdirectory(t03_cube)
add_subdirectory(t04_motion)
add_subdirectory(t05_two_cubes)
add_subdirectory(t06_phong_model)
add_subdirectory(t07_lighting_map)
add_subdirectory(t08_instancing)
add_subdirectory(t09_load_mesh)
add_subdirectory(t10_blending)
add_subdirectory(t11_framebuffer)
add_subdirectory(t12_cubemaps)
