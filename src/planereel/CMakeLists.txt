set(PLANEREEL_SOURCES
  main.cc
  )

add_executable(planereel ${PLANEREEL_SOURCES})
add_dependencies(planereel notcurses++)
target_link_libraries(planereel PRIVATE notcurses++)
