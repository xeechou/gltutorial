include_directories(
  ${GLFW_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIR}
  ${assimp_INCLUDE_DIRS}
  )

add_executable(tt12
  maintest.cpp
  )

target_link_libraries(tt12
  ${LIB_3rdParties}
  common
  drawobjcollects
  )
