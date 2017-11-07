add_executable(facialTest
  faciallandmarks.cpp)

target_link_libraries(facialTest
  ${DLIB_LIBRARIES}
  ${OpenCV_LIBS}  
  )
