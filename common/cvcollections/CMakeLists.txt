add_executable(facialTest
  faciallandmarks.cpp)

target_link_libraries(facialTest
  ${DLIB_LIBRARIES}
  ${OpenCV_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  )

target_compile_options(facialTest
  PRIVATE "-msse4"
  )

add_executable(threadTest
  threadt.cpp
  )

target_link_libraries(threadTest
  ${OpenCV_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  )

#get_target_property(pppp facialTest COMPILE_OPTIONS)
#message("facial property: ${pppp}")
