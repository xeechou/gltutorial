include_directories(
  ${GLFW_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIR}
  ${assimp_INCLUDE_DIRS}
  )


add_executable(t9
  main.cc
  )

target_link_libraries(t9
  ${GLFW_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GLM_LIBRARIES}
  ${OPENGL_LIBRARY}
  ${OpenCV_LIBS}
  ${assimp_LIBRARIES}
  common)
