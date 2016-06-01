include_directories(
  ${GLFW_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  )


add_executable(t4
  main.cc
  controls.cpp
  )

target_link_libraries(t4
  ${GLFW_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GLM_LIBRARIES}
  ${OPENGL_LIBRARY}
  common)
