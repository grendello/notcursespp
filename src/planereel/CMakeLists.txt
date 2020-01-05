set(PLANEREEL_SOURCES
  main.cc
  )

add_executable(ncpp-planereel ${PLANEREEL_SOURCES})
add_dependencies(ncpp-planereel notcurses++)
target_link_libraries(ncpp-planereel PRIVATE notcurses++)
install(TARGETS ncpp-planereel DESTINATION bin)
