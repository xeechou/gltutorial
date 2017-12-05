cmake_minimum_required(VERSION 3.1.0)

project(mysamples
  VERSION 0.1
  LANGUAGES C CXX)

set(CMAKE_C_FLAGS "-g")
add_definitions(
  -D_GNU_SOURCE
  )
if(MSYS)
  add_definitions(-D_GLEW_STATIC)
endif()


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC")
set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -g -fPIC")
#last time I check, the target_compile_options only add additional options, it doesn't override everything, so I am good
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/cmake")
if (WIN32)
   set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/buildWin")
else()
   set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(PkgConfig REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(glm   REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
#dlib is purely headerfile, but we need to include the dlib CMakeLists. And include
find_package(DLIB REQUIRED)
#find_package(assimp REQUIRED) this doesn't work on windows either
#find_package(GLFW REQUIRED)

pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(EIGEN3 REQUIRED eigen3)
pkg_search_module(GLEW REQUIRED glew)
pkg_search_module(assimp REQUIRED assimp)
#for compatibility with mingw"
include_directories(${EIGEN3_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(include)


set(LIB_3rdParties
  ${GLFW_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GLM_LIBRARIES}
  ${OPENGL_LIBRARY}
  ${OpenCV_LIBS}
  ${assimp_LIBRARIES}
  )

set(LIB_3rdPartiesCV
  ${OpenCV_LIBS}
  ${DLIB_LIBRARIES}
  )

message("build with compiler flags:  " ${CMAKE_CXX_FLAGS})

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
#add_subdirectory(t09_load_mesh)
#add_subdirectory(t10_blending)
add_subdirectory(t11_framebuffer)
add_subdirectory(t12_cubemaps)
add_subdirectory(t13_lightmap)
add_subdirectory(t14_boneAnime)
add_subdirectory(t15_simple_shaders)
