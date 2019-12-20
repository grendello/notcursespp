set(UNIDAMAGE_SOURCES
  unidamage.cc
  )

add_executable(unidamage ${UNIDAMAGE_SOURCES})
add_dependencies(unidamage notcurses++)
target_link_libraries(unidamage PRIVATE notcurses++)


set(RGB_SOURCES
  rgb.cc
  )

add_executable(rgb ${RGB_SOURCES})
add_dependencies(rgb notcurses++)
target_link_libraries(rgb PRIVATE notcurses++)


set(RGB_SOURCES
  rgbbg.cc
  )

add_executable(rgbbg ${RGB_SOURCES})
add_dependencies(rgbbg notcurses++)
target_link_libraries(rgbbg PRIVATE notcurses++)
