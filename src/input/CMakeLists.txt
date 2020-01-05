set(INPUT_SOURCES
  input.cc
  )

add_executable(ncpp-input ${INPUT_SOURCES})
add_dependencies(ncpp-input notcurses++)
target_link_libraries(ncpp-input PRIVATE notcurses++)
install(TARGETS ncpp-input DESTINATION bin)
