include_directories(
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  )

add_library(common STATIC
  utils.cpp
  controls.cpp
  shaderman.cpp
  model.cpp
  framebuffer.cpp
  context.cpp
  data.cpp
  )

target_link_libraries(common
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${assimp_LIBRARIES})
