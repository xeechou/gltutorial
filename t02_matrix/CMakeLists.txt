include_directories(
  ${GLFW_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  )


add_executable(t2
  main.cc
  )

target_link_libraries(t2
  ${GLFW_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GLM_LIBRARIES}
  ${OPENGL_LIBRARY}
  common)
