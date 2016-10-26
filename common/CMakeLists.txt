include_directories(
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  )

add_library(common STATIC
  utils.cpp
  controls.cpp
  shaderman.cpp
  mesh.cpp)

target_link_libraries(common
  ${assimp_LIBRARIES})
