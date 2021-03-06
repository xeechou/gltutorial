include_directories(
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  ${EIGEN3_INCLUDE_DIR}
  ${DLIB_INCLUDE_DIR}
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
  geometry.cpp
  collections/mesh.cpp
  collections/material.cpp
  collections/instancing.cpp
  collections/bone.cpp
  collections/transforming.cpp
  )

target_link_libraries(common
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${assimp_LIBRARIES}
  ${OPENGL_gl_LIBRARY}
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}
  )

add_library(shadercollects STATIC
  collections/basic_shaders.cpp)

target_link_libraries(shadercollects
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${assimp_LIBRARIES}
  ${OPENGL_gl_LIBRARY}
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}
  )

add_library(drawobjcollects STATIC
  collections/cubemap.cpp)

target_link_libraries(shadercollects
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${assimp_LIBRARIES}
  ${OPENGL_gl_LIBRARY}
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}
  )
add_subdirectory(cvcollections)
