include_directories(
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  ${EIGEN3_INCLUDE_DIR}
  )

add_library(common STATIC
  utils.cpp
  tree.cpp  
  controls.cpp
  shaderman.cpp
  framebuffer.cpp
  context.cpp
  data.cpp
  #model loaders,
  model.cpp
  mesh.cpp
  bone.cpp
  geometry.cpp
  )

target_link_libraries(common
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${assimp_LIBRARIES}
  ${OPENGL_gl_LIBRARY}
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}
  )


add_library(shadercollect STATIC
  collections/basic_shaders.cpp)

target_link_libraries(shadercollect
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${assimp_LIBRARIES}
  ${OPENGL_gl_LIBRARY}
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}
  )
