set(UNIDAMAGE_SOURCES
  unidamage.cc
  )

add_executable(unidamage ${UNIDAMAGE_SOURCES})
add_dependencies(unidamage notcurses++)
target_link_libraries(unidamage PRIVATE notcurses++)
