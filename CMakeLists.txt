cmake_minimum_required(VERSION 3.1.0)

project(mysamples
  VERSION 0.1
  LANGUAGES C CXX)

set(CMAKE_C_FLAGS "-g")
add_definitions(
  -D_GNU_SOURCE
  )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/CMake")

find_package(PkgConfig REQUIRED)

find_package(GLEW  REQUIRED)
find_package(glm   REQUIRED)
find_package(OpenGL REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(include)
add_subdirectory(common)

#tutorial folders
add_subdirectory(t0_triangle)
add_subdirectory(t2_matrix)
add_subdirectory(t3_cube)
