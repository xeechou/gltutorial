add_executable(facialTest
  faciallandmarks.cpp)

target_link_libraries(facialTest
  ${DLIB_LIBRARIES}
  ${OpenCV_LIBS}
  )

target_compile_options(facialTest
  PRIVATE "-msse4"
  )

add_executable(threadTest
  threadt.cpp
  )

target_link_libraries(threadTest
  ${OpenCV_LIBS})

#get_target_property(pppp facialTest COMPILE_OPTIONS)
#message("facial property: ${pppp}")
