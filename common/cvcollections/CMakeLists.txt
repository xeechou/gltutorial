add_executable(facialTest
  faciallandmarks.cpp)

target_link_libraries(facialTest
  ${DLIB_LIBRARIES}
  ${OpenCV_LIBS}
  )

target_compile_options(facialTest
  PRIVATE "-mavx"
  PRIVATE "-msse4"
  )

#get_target_property(pppp facialTest COMPILE_OPTIONS)
#message("facial property: ${pppp}")
