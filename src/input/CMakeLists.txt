set(INPUT_SOURCES
  input.cc
  )

add_executable(input ${INPUT_SOURCES})
add_dependencies(input notcurses++)
target_link_libraries(input PRIVATE notcurses++)
